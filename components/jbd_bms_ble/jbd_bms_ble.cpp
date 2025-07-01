#include "jbd_bms_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace jbd_bms_ble {

static const char *const TAG = "jbd_bms_ble";

static const uint8_t MAX_NO_RESPONSE_COUNT = 10;

static const uint16_t JBD_BMS_SERVICE_UUID = 0xFF00;
static const uint16_t JBD_BMS_NOTIFY_CHARACTERISTIC_UUID = 0xFF01;
static const uint16_t JBD_BMS_CONTROL_CHARACTERISTIC_UUID = 0xFF02;

static const uint16_t MAX_RESPONSE_SIZE = 41;

static const uint8_t JBD_PKT_START = 0xDD;
static const uint8_t JBD_PKT_END = 0x77;
static const uint8_t JBD_CMD_READ = 0xA5;
static const uint8_t JBD_CMD_WRITE = 0x5A;

static const uint8_t JBD_AUTH_PKT_START = 0xFF;
static const uint8_t JBD_AUTH_PKT_SECOND = 0xAA;
static const uint8_t JBD_AUTH_PKT_END = 0x77;

static const uint8_t JBD_AUTH_SEND_APP_KEY = 0x15;
static const uint8_t JBD_AUTH_GET_RANDOM = 0x17;
static const uint8_t JBD_AUTH_SEND_PASSWORD = 0x18;
static const uint8_t JBD_AUTH_CHANGE_PASSWORD = 0x16;
static const uint8_t JBD_AUTH_SEND_ROOT_PASSWORD = 0x1D;

static const uint8_t JBD_CMD_HWINFO = 0x03;
static const uint8_t JBD_CMD_CELLINFO = 0x04;
static const uint8_t JBD_CMD_HWVER = 0x05;

static const uint8_t JBD_CMD_ENTER_FACTORY = 0x00;
static const uint8_t JBD_CMD_EXIT_FACTORY = 0x01;
static const uint8_t JBD_CMD_FORCE_SOC_RESET = 0x0A;
static const uint8_t JBD_CMD_ERROR_COUNTS = 0xAA;
static const uint8_t JBD_CMD_CAP_REM = 0xE0;   // Set remaining capacity
static const uint8_t JBD_CMD_MOS = 0xE1;       // Set charging/discharging bitmask
static const uint8_t JBD_CMD_BALANCER = 0xE2;  // Enable/disable balancer

static const uint8_t JBD_MOS_CHARGE = 0x01;
static const uint8_t JBD_MOS_DISCHARGE = 0x02;

static const uint8_t ERRORS_SIZE = 16;
static const char *const ERRORS[ERRORS_SIZE] = {
    "Cell overvoltage",               // 0x00
    "Cell undervoltage",              // 0x01
    "Pack overvoltage",               // 0x02
    "Pack undervoltage",              // 0x03
    "Charging over temperature",      // 0x04
    "Charging under temperature",     // 0x05
    "Discharging over temperature",   // 0x06
    "Discharging under temperature",  // 0x07
    "Charging overcurrent",           // 0x08
    "Discharging overcurrent",        // 0x09
    "Short circuit",                  // 0x0A
    "IC front-end error",             // 0x0B
    "Mosfet Software Lock",           // 0x0C (See register 0xE1 "MOSFET control")
    "Charge timeout Close",           // 0x0D
    "Unknown (0x0E)",                 // 0x0E
    "Unknown (0x0F)",                 // 0x0F
};

static const uint8_t ROOT_PASSWORD[] = {0x4a, 0x42, 0x44, 0x62, 0x74, 0x70, 0x77, 0x64,
                                        0x21, 0x40, 0x23, 0x32, 0x30, 0x32, 0x33};
static const size_t ROOT_PASSWORD_LENGTH = sizeof(ROOT_PASSWORD);

void JbdBmsBle::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                    esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;

      if (this->char_notify_handle_ != 0) {
        auto status = esp_ble_gattc_unregister_for_notify(this->parent()->get_gattc_if(),
                                                          this->parent()->get_remote_bda(), this->char_notify_handle_);
        if (status) {
          ESP_LOGW(TAG, "esp_ble_gattc_unregister_for_notify failed, status=%d", status);
        }
      }
      this->char_notify_handle_ = 0;
      this->char_command_handle_ = 0;

      this->frame_buffer_.clear();
      this->authentication_state_ = AuthState::NOT_AUTHENTICATED;
      this->random_byte_ = 0;

      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      auto *char_notify = this->parent_->get_characteristic(JBD_BMS_SERVICE_UUID, JBD_BMS_NOTIFY_CHARACTERISTIC_UUID);
      if (char_notify == nullptr) {
        ESP_LOGE(TAG, "[%s] No notify service found at device, not an JBD BMS..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->char_notify_handle_ = char_notify->handle;

      auto status = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                                                      char_notify->handle);
      if (status) {
        ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
      }

      auto *char_command = this->parent_->get_characteristic(JBD_BMS_SERVICE_UUID, JBD_BMS_CONTROL_CHARACTERISTIC_UUID);
      if (char_command == nullptr) {
        ESP_LOGE(TAG, "[%s] No control service found at device, not an JBD BMS..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->char_command_handle_ = char_command->handle;
      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      this->node_state = espbt::ClientState::ESTABLISHED;

      if (this->enable_authentication_) {
        this->start_authentication_();
      } else {
        this->send_command(JBD_CMD_READ, JBD_CMD_HWINFO);
      }

      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.handle != this->char_notify_handle_)
        break;

      ESP_LOGV(TAG, "Notification received (handle 0x%02X): %s", param->notify.handle,
               format_hex_pretty(param->notify.value, param->notify.value_len).c_str());

      this->assemble(param->notify.value, param->notify.value_len);

      break;
    }
    default:
      break;
  }
}

void JbdBmsBle::start_authentication_() {
  ESP_LOGI(TAG, "Starting authentication flow");
  this->authentication_state_ = AuthState::SENDING_APP_KEY;
  this->auth_timeout_start_ = millis();
  this->send_app_key_();
}

void JbdBmsBle::send_app_key_() {
  ESP_LOGD(TAG, "Sending app key");

  uint8_t frame[11];
  frame[0] = JBD_AUTH_PKT_START;
  frame[1] = JBD_AUTH_PKT_SECOND;
  frame[2] = JBD_AUTH_SEND_APP_KEY;
  frame[3] = 0x06;
  frame[4] = 0x30;
  frame[5] = 0x30;
  frame[6] = 0x30;  // "000000"
  frame[7] = 0x30;
  frame[8] = 0x30;
  frame[9] = 0x30;
  frame[10] = auth_chksum_(frame + 2, 8);

  this->send_auth_frame_(frame, sizeof(frame));
}

void JbdBmsBle::request_random_byte_() {
  ESP_LOGD(TAG, "Requesting random byte");

  uint8_t frame[5];
  frame[0] = JBD_AUTH_PKT_START;
  frame[1] = JBD_AUTH_PKT_SECOND;
  frame[2] = JBD_AUTH_GET_RANDOM;
  frame[3] = 0x00;
  frame[4] = auth_chksum_(frame + 2, 2);

  this->send_auth_frame_(frame, sizeof(frame));
}

void JbdBmsBle::send_user_password_() {
  ESP_LOGD(TAG, "Sending encrypted user password with random byte: 0x%02X", this->random_byte_);

  uint8_t *remote_bda = this->parent()->get_remote_bda();

  // Use configured password or default to "123123" if empty
  std::string password_str = this->password_.empty() ? "123123" : this->password_;

  uint8_t frame[11];
  frame[0] = JBD_AUTH_PKT_START;
  frame[1] = JBD_AUTH_PKT_SECOND;
  frame[2] = JBD_AUTH_SEND_PASSWORD;
  frame[3] = 0x06;

  for (int i = 0; i < 6; i++) {
    frame[4 + i] = ((remote_bda[i] ^ static_cast<uint8_t>(password_str[i])) + this->random_byte_) & 255;
  }

  frame[10] = auth_chksum_(frame + 2, 8);

  this->send_auth_frame_(frame, sizeof(frame));
}

void JbdBmsBle::send_root_password_() {
  ESP_LOGD(TAG, "Sending encrypted root password with random byte: 0x%02X", this->random_byte_);

  uint8_t *remote_bda = this->parent()->get_remote_bda();
  uint8_t encrypted[ROOT_PASSWORD_LENGTH];

  // Encrypt root password using MAC address and random byte
  for (size_t i = 0; i < ROOT_PASSWORD_LENGTH; i++) {
    uint8_t mac_byte = (i < 6) ? remote_bda[i] : 0x00;  // Use 0x00 for bytes beyond MAC length
    encrypted[i] = ((mac_byte ^ ROOT_PASSWORD[i]) + this->random_byte_) & 255;
  }

  uint8_t frame[20];  // 4 + 15 + 1
  frame[0] = JBD_AUTH_PKT_START;
  frame[1] = JBD_AUTH_PKT_SECOND;
  frame[2] = JBD_AUTH_SEND_ROOT_PASSWORD;
  frame[3] = ROOT_PASSWORD_LENGTH;

  for (size_t i = 0; i < ROOT_PASSWORD_LENGTH; i++) {
    frame[4 + i] = encrypted[i];
  }

  frame[4 + ROOT_PASSWORD_LENGTH] = auth_chksum_(frame + 2, 2 + ROOT_PASSWORD_LENGTH);

  this->send_auth_frame_(frame, 5 + ROOT_PASSWORD_LENGTH);
}

void JbdBmsBle::send_auth_frame_(uint8_t *frame, size_t length) {
  ESP_LOGV(TAG, "Send auth frame (handle 0x%02X): %s", this->char_command_handle_,
           format_hex_pretty(frame, length).c_str());

  auto status =
      esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_command_handle_,
                               length, frame, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);
  }
}

void JbdBmsBle::assemble(const uint8_t *data, uint16_t length) {
  if (this->frame_buffer_.size() > MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Maximum response size exceeded");
    this->frame_buffer_.clear();
  }

  // Auth frames (0xFF 0xAA) are complete in single BLE notification, process directly
  if (length >= 5 && data[0] == JBD_AUTH_PKT_START && data[1] == JBD_AUTH_PKT_SECOND) {
    uint8_t command = data[2];
    uint8_t data_len = data[3];
    uint8_t expected_frame_len = 4 + data_len + 1;  // FF AA CMD LEN DATA... CHECKSUM

    if (length >= expected_frame_len) {
      uint8_t computed_crc = auth_chksum_(data + 2, 2 + data_len);
      uint8_t remote_crc = data[4 + data_len];

      if (computed_crc == remote_crc) {
        this->handle_auth_response_(command, data + 4, data_len);
      } else {
        ESP_LOGW(TAG, "Auth frame checksum failed! 0x%02X != 0x%02X", computed_crc, remote_crc);
      }
    }
    return;
  }

  // Regular BMS frames need assembly
  bool is_new_frame = false;
  if (length >= 3 && data[0] == JBD_PKT_START && data[2] == 0x00) {
    is_new_frame = true;
  }

  if (is_new_frame) {
    this->frame_buffer_.clear();
  }

  this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);

  if (this->frame_buffer_.size() >= 7 && this->frame_buffer_[0] == JBD_PKT_START &&
      this->frame_buffer_.back() == JBD_PKT_END) {
    const uint8_t *raw = &this->frame_buffer_[0];
    uint8_t function = raw[1];
    uint16_t data_len = raw[3];
    uint16_t frame_len = 4 + data_len + 3;

    if (frame_len == this->frame_buffer_.size()) {
      uint16_t computed_crc = chksum_(raw + 2, data_len + 2);
      uint16_t remote_crc = uint16_t(raw[frame_len - 3]) << 8 | (uint16_t(raw[frame_len - 2]) << 0);

      if (computed_crc == remote_crc) {
        std::vector<uint8_t> frame_data(this->frame_buffer_.begin() + 4, this->frame_buffer_.end() - 3);
        this->on_jbd_bms_data(function, frame_data);
      } else {
        ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, remote_crc);
      }
    } else {
      ESP_LOGW(TAG, "Invalid frame length: expected %d, got %d", frame_len, this->frame_buffer_.size());
    }
    this->frame_buffer_.clear();
  }
}

void JbdBmsBle::handle_auth_response_(uint8_t command, const uint8_t *data, uint8_t data_len) {
  ESP_LOGV(TAG, "Auth response - Command: 0x%02X, Data len: %d", command, data_len);

  switch (command) {
    case JBD_AUTH_SEND_APP_KEY:
      switch (data[0]) {
        case 0x00:  // App key accepted, password required
          ESP_LOGD(TAG, "App key accepted, password required - requesting random byte");
          this->authentication_state_ = AuthState::REQUESTING_RANDOM;
          this->request_random_byte_();
          break;
        case 0x02:  // No password set, direct access
          ESP_LOGI(TAG, "App key accepted, no password required - authentication complete");
          this->authentication_state_ = AuthState::AUTHENTICATED;
          // Start normal data collection immediately
          this->send_command(JBD_CMD_READ, JBD_CMD_HWINFO);
          break;
        case 0x01:  // App key rejected
          ESP_LOGE(TAG, "App key rejected");
          this->authentication_state_ = AuthState::NOT_AUTHENTICATED;
          break;
        default:
          ESP_LOGW(TAG, "Unknown app key response: 0x%02X", data[0]);
          this->authentication_state_ = AuthState::NOT_AUTHENTICATED;
          break;
      }
      break;

    case JBD_AUTH_GET_RANDOM:
      this->random_byte_ = data[0];
      ESP_LOGD(TAG, "Received random byte: 0x%02X", this->random_byte_);
      if (this->authentication_state_ == AuthState::REQUESTING_RANDOM) {
        this->authentication_state_ = AuthState::SENDING_PASSWORD;
        this->send_user_password_();
      } else if (this->authentication_state_ == AuthState::REQUESTING_ROOT_RANDOM) {
        this->authentication_state_ = AuthState::SENDING_ROOT_PASSWORD;
        this->send_root_password_();
      }
      break;

    case JBD_AUTH_SEND_PASSWORD:
      if (data[0] == 0x00) {  // Success
        ESP_LOGD(TAG, "Password accepted, requesting new random byte for root password");
        this->authentication_state_ = AuthState::REQUESTING_ROOT_RANDOM;
        this->request_random_byte_();
      } else {
        ESP_LOGE(TAG, "Password rejected");
        this->authentication_state_ = AuthState::NOT_AUTHENTICATED;
      }
      break;

    case JBD_AUTH_SEND_ROOT_PASSWORD:
      if (data[0] == 0x00) {  // Success
        ESP_LOGI(TAG, "Authentication successful!");
        this->authentication_state_ = AuthState::AUTHENTICATED;
        // Start normal data collection
        this->send_command(JBD_CMD_READ, JBD_CMD_HWINFO);
      } else {
        ESP_LOGE(TAG, "Root password rejected");
        this->authentication_state_ = AuthState::NOT_AUTHENTICATED;
      }
      break;

    default:
      ESP_LOGW(TAG, "Unknown auth command: 0x%02X", command);
      break;
  }
}

void JbdBmsBle::update() {
  this->track_online_status_();
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
    return;
  }

  if (this->enable_authentication_ && this->authentication_state_ != AuthState::AUTHENTICATED) {
    if (this->authentication_state_ == AuthState::NOT_AUTHENTICATED) {
      this->start_authentication_();
    } else {
      this->check_auth_timeout_();
      ESP_LOGV(TAG, "[%s] Not authenticated yet", this->parent_->address_str().c_str());
    }
    return;
  }

  this->send_command(JBD_CMD_READ, JBD_CMD_HWINFO);
}

void JbdBmsBle::check_auth_timeout_() {
  if (this->authentication_state_ == AuthState::NOT_AUTHENTICATED ||
      this->authentication_state_ == AuthState::AUTHENTICATED) {
    return;
  }

  const uint32_t now = millis();
  if (now - this->auth_timeout_start_ > this->auth_timeout_ms_) {
    ESP_LOGW(TAG, "[%s] Authentication timeout after %d ms, resetting to retry", this->parent_->address_str().c_str(),
             this->auth_timeout_ms_);
    this->authentication_state_ = AuthState::NOT_AUTHENTICATED;
  }
}

void JbdBmsBle::on_jbd_bms_data(const uint8_t &function, const std::vector<uint8_t> &data) {
  this->reset_online_status_tracker_();

  switch (function) {
    case JBD_CMD_HWINFO:
      this->on_hardware_info_data_(data);
      this->send_command(JBD_CMD_READ, JBD_CMD_CELLINFO);
      break;
    case JBD_CMD_CELLINFO:
      this->on_cell_info_data_(data);
      break;
    case JBD_CMD_HWVER:
      this->on_hardware_version_data_(data);
      break;
    case JBD_CMD_ERROR_COUNTS:
      this->on_error_counts_data_(data);
      break;
    case JBD_CMD_MOS:
    case JBD_CMD_EXIT_FACTORY:
    case JBD_CMD_FORCE_SOC_RESET:
      break;
    default:
      ESP_LOGW(TAG, "Unhandled response (function 0x%02X) received: %s", function,
               format_hex_pretty(&data.front(), data.size()).c_str());
  }
}

void JbdBmsBle::on_cell_info_data_(const std::vector<uint8_t> &data) {
  auto jbd_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Cell info frame (%d bytes) received", data.size());
  ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  uint8_t data_len = data.size();
  if (data_len < 2 || data_len > 64 || (data_len % 2) != 0) {
    ESP_LOGW(TAG, "Skipping cell info frame because of invalid length: %d", data_len);
    return;
  }

  // Byte Len  Payload                Content              Coeff.      Unit        Example value
  // 0     2   0x0F 0x3B              Cell voltage 1       0.001       V           3899 = 3.899
  // 2     2   0x0F 0x30              Cell voltage 2       0.001       V
  // 4     2   0x0F 0x2D              Cell voltage 3       0.001       V
  // 6     2   0x0F 0x31              Cell voltage 4       0.001       V
  uint8_t cells = std::min(data_len / 2, 32);
  float min_cell_voltage = 100.0f;
  float max_cell_voltage = -100.0f;
  float average_cell_voltage = 0.0f;
  uint8_t min_voltage_cell = 0;
  uint8_t max_voltage_cell = 0;
  for (uint8_t i = 0; i < cells; i++) {
    float cell_voltage = (float) jbd_get_16bit(0 + (i * 2)) * 0.001f;
    average_cell_voltage = average_cell_voltage + cell_voltage;
    if (cell_voltage < min_cell_voltage) {
      min_cell_voltage = cell_voltage;
      min_voltage_cell = i + 1;
    }
    if (cell_voltage > max_cell_voltage) {
      max_cell_voltage = cell_voltage;
      max_voltage_cell = i + 1;
    }
    this->publish_state_(this->cells_[i].cell_voltage_sensor_, cell_voltage);
  }
  average_cell_voltage = average_cell_voltage / cells;

  this->publish_state_(this->min_cell_voltage_sensor_, min_cell_voltage);
  this->publish_state_(this->max_cell_voltage_sensor_, max_cell_voltage);
  this->publish_state_(this->max_voltage_cell_sensor_, (float) max_voltage_cell);
  this->publish_state_(this->min_voltage_cell_sensor_, (float) min_voltage_cell);
  this->publish_state_(this->delta_cell_voltage_sensor_, max_cell_voltage - min_cell_voltage);
  this->publish_state_(this->average_cell_voltage_sensor_, average_cell_voltage);
}

void JbdBmsBle::on_hardware_info_data_(const std::vector<uint8_t> &data) {
  auto jbd_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };
  auto jbd_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(jbd_get_16bit(i + 0)) << 16) | (uint32_t(jbd_get_16bit(i + 2)) << 0);
  };

  ESP_LOGI(TAG, "Hardware info frame (%d bytes) received", data.size());
  ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  ESP_LOGD(TAG, "  Device model: %s", this->device_model_.c_str());

  // Byte Len  Payload                Content              Coeff.      Unit        Example value
  //  0    2   0x06 0x17              Total voltage                                1559
  float total_voltage = jbd_get_16bit(0) * 0.01f;
  this->publish_state_(this->total_voltage_sensor_, total_voltage);

  //  2    2   0x00 0x00              Current
  float current = (float) ((int16_t) jbd_get_16bit(2)) * 0.01f;
  float power = total_voltage * current;
  this->publish_state_(this->current_sensor_, current);
  this->publish_state_(this->power_sensor_, power);
  this->publish_state_(this->charging_power_sensor_, std::max(0.0f, power));               // 500W vs 0W -> 500W
  this->publish_state_(this->discharging_power_sensor_, std::abs(std::min(0.0f, power)));  // -500W vs 0W -> 500W

  //  4    2   0x01 0xF3              Residual Capacity                            499
  this->publish_state_(this->capacity_remaining_sensor_, (float) jbd_get_16bit(4) * 0.01f);

  //  6    2   0x01 0xF4              Nominal Capacity                             500
  this->publish_state_(this->nominal_capacity_sensor_, (float) jbd_get_16bit(6) * 0.01f);

  //  8    2   0x00 0x00              Cycle Life
  this->publish_state_(this->charging_cycles_sensor_, (float) jbd_get_16bit(8));

  // 10    2   0x2C 0x7C              Production date
  uint16_t production_date = jbd_get_16bit(10);
  ESP_LOGD(TAG, "  Date of manufacture: %d.%d.%d", 2000 + (production_date >> 9), (production_date >> 5) & 0x0f,
           production_date & 0x1f);

  // 12    4   0x00 0x00 0x00 0x00    Balancer Status
  uint32_t balance_status_bitmask = jbd_get_32bit(12);
  this->publish_state_(this->balancer_status_bitmask_sensor_, (float) balance_status_bitmask);
  this->publish_state_(this->balancing_binary_sensor_, balance_status_bitmask > 0);

  // 16    2   0x00 0x00              Protection Status
  uint16_t errors_bitmask = jbd_get_16bit(16);
  this->publish_state_(this->errors_bitmask_sensor_, (float) errors_bitmask);
  this->publish_state_(this->errors_text_sensor_, this->error_bits_to_string_(errors_bitmask));

  // 18    1   0x80                   Version                                      0x10 = 1.0, 0x80 = 8.0
  this->publish_state_(this->software_version_sensor_, (data[18] >> 4) + ((data[18] & 0x0f) * 0.1f));

  // 19    1   0x64                   State of charge
  this->publish_state_(this->state_of_charge_sensor_, data[19]);

  // 20    1   0x03                   Mosfet bitmask
  uint8_t operation_status = data[20];
  this->mosfet_status_ = operation_status;
  this->publish_state_(this->operation_status_bitmask_sensor_, operation_status);
  this->publish_state_(this->charging_binary_sensor_, operation_status & JBD_MOS_CHARGE);
  this->publish_state_(this->charging_switch_, operation_status & JBD_MOS_CHARGE);
  this->publish_state_(this->discharging_binary_sensor_, operation_status & JBD_MOS_DISCHARGE);
  this->publish_state_(this->discharging_switch_, operation_status & JBD_MOS_DISCHARGE);

  // 21    2   0x04                   Cell count
  this->publish_state_(this->battery_strings_sensor_, data[21]);

  // 22    3   0x03                   Temperature sensors
  // 23    2   0x0B 0x8D              Temperature 1
  // 25    2   0x0B 0x8C              Temperature 2
  // 27    2   0x0B 0x88              Temperature 3
  uint8_t temperature_sensors = std::min(data[22], (uint8_t) 6);
  this->publish_state_(this->temperature_sensors_sensor_, temperature_sensors);
  for (uint8_t i = 0; i < temperature_sensors; i++) {
    this->publish_state_(this->temperatures_[i].temperature_sensor_,
                         (float) (jbd_get_16bit(23 + (i * 2)) - 2731) * 0.1f);
  }
}

void JbdBmsBle::on_error_counts_data_(const std::vector<uint8_t> &data) {
  auto jbd_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };

  ESP_LOGI(TAG, "Error counts frame (%d bytes) received", data.size());
  ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  uint8_t data_len = data.size();
  if (data_len != 24) {
    ESP_LOGW(TAG, "Skipping error counts frame because of invalid length: %d", data_len);
    return;
  }

  this->publish_state_(this->short_circuit_error_count_sensor_, jbd_get_16bit(0));
  this->publish_state_(this->charge_overcurrent_error_count_sensor_, jbd_get_16bit(2));
  this->publish_state_(this->discharge_overcurrent_error_count_sensor_, jbd_get_16bit(4));
  this->publish_state_(this->cell_overvoltage_error_count_sensor_, jbd_get_16bit(6));
  this->publish_state_(this->cell_undervoltage_error_count_sensor_, jbd_get_16bit(8));
  this->publish_state_(this->charge_overtemperature_error_count_sensor_, jbd_get_16bit(10));
  this->publish_state_(this->charge_undertemperature_error_count_sensor_, jbd_get_16bit(12));
  this->publish_state_(this->discharge_overtemperature_error_count_sensor_, jbd_get_16bit(14));
  this->publish_state_(this->discharge_undertemperature_error_count_sensor_, jbd_get_16bit(16));
  this->publish_state_(this->battery_overvoltage_error_count_sensor_, jbd_get_16bit(18));
  this->publish_state_(this->battery_undervoltage_error_count_sensor_, jbd_get_16bit(20));
}

void JbdBmsBle::on_hardware_version_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Hardware version frame (%d bytes) received", data.size());
  ESP_LOGVV(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Byte Len  Payload                                              Content
  // 0    25   0x4A 0x42 0x44 0x2D 0x53 0x50 0x30 0x34 0x53 0x30
  //           0x33 0x34 0x2D 0x4C 0x34 0x53 0x2D 0x32 0x30 0x30
  //           0x41 0x2D 0x42 0x2D 0x55
  this->device_model_ = std::string(data.begin(), data.end());

  ESP_LOGI(TAG, "  Model name: %s", this->device_model_.c_str());
  this->publish_state_(this->device_model_text_sensor_, this->device_model_);
}

void JbdBmsBle::track_online_status_() {
  if (this->no_response_count_ < MAX_NO_RESPONSE_COUNT) {
    this->no_response_count_++;
  }
  if (this->no_response_count_ == MAX_NO_RESPONSE_COUNT) {
    this->publish_device_unavailable_();
    this->no_response_count_++;
  }
}

void JbdBmsBle::reset_online_status_tracker_() {
  this->no_response_count_ = 0;
  this->publish_state_(this->online_status_binary_sensor_, true);
}

void JbdBmsBle::publish_device_unavailable_() {
  this->publish_state_(this->online_status_binary_sensor_, false);
  this->publish_state_(this->errors_text_sensor_, "Offline");

  this->publish_state_(state_of_charge_sensor_, NAN);
  this->publish_state_(total_voltage_sensor_, NAN);
  this->publish_state_(current_sensor_, NAN);
  this->publish_state_(power_sensor_, NAN);
  this->publish_state_(charging_power_sensor_, NAN);
  this->publish_state_(discharging_power_sensor_, NAN);
  this->publish_state_(nominal_capacity_sensor_, NAN);
  this->publish_state_(charging_cycles_sensor_, NAN);
  this->publish_state_(capacity_remaining_sensor_, NAN);
  this->publish_state_(min_cell_voltage_sensor_, NAN);
  this->publish_state_(max_cell_voltage_sensor_, NAN);
  this->publish_state_(min_voltage_cell_sensor_, NAN);
  this->publish_state_(max_voltage_cell_sensor_, NAN);
  this->publish_state_(delta_cell_voltage_sensor_, NAN);
  this->publish_state_(average_cell_voltage_sensor_, NAN);
  this->publish_state_(operation_status_bitmask_sensor_, NAN);
  this->publish_state_(errors_bitmask_sensor_, NAN);
  this->publish_state_(balancer_status_bitmask_sensor_, NAN);
  this->publish_state_(battery_strings_sensor_, NAN);
  this->publish_state_(temperature_sensors_sensor_, NAN);
  this->publish_state_(software_version_sensor_, NAN);

  for (auto &temperature : this->temperatures_) {
    this->publish_state_(temperature.temperature_sensor_, NAN);
  }

  for (auto &cell : this->cells_) {
    this->publish_state_(cell.cell_voltage_sensor_, NAN);
  }
}

void JbdBmsBle::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "JbdBmsBle:");

  LOG_BINARY_SENSOR("", "Balancing", this->balancing_binary_sensor_);
  LOG_BINARY_SENSOR("", "Charging", this->charging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Discharging", this->discharging_binary_sensor_);

  LOG_SENSOR("", "Total voltage", this->total_voltage_sensor_);
  LOG_SENSOR("", "Battery strings", this->battery_strings_sensor_);
  LOG_SENSOR("", "Software version", this->software_version_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Charging Power", this->charging_power_sensor_);
  LOG_SENSOR("", "Discharging Power", this->discharging_power_sensor_);
  LOG_SENSOR("", "State of charge", this->state_of_charge_sensor_);
  LOG_SENSOR("", "Operation status bitmask", operation_status_bitmask_sensor_);
  LOG_SENSOR("", "Errors bitmask", errors_bitmask_sensor_);
  LOG_SENSOR("", "Nominal capacity", this->nominal_capacity_sensor_);
  LOG_SENSOR("", "Charging cycles", this->charging_cycles_sensor_);
  LOG_SENSOR("", "Balancer status bitmask", balancer_status_bitmask_sensor_);
  LOG_SENSOR("", "Capacity remaining", this->capacity_remaining_sensor_);
  LOG_SENSOR("", "Average cell voltage sensor", this->average_cell_voltage_sensor_);
  LOG_SENSOR("", "Delta cell voltage sensor", delta_cell_voltage_sensor_);
  LOG_SENSOR("", "Maximum cell voltage", this->max_cell_voltage_sensor_);
  LOG_SENSOR("", "Min voltage cell", this->min_voltage_cell_sensor_);
  LOG_SENSOR("", "Max voltage cell", this->max_voltage_cell_sensor_);
  LOG_SENSOR("", "Minimum cell voltage", this->min_cell_voltage_sensor_);
  LOG_SENSOR("", "Temperature sensors", temperature_sensors_sensor_);
  LOG_SENSOR("", "Temperature 1", this->temperatures_[0].temperature_sensor_);
  LOG_SENSOR("", "Temperature 2", this->temperatures_[1].temperature_sensor_);
  LOG_SENSOR("", "Temperature 3", this->temperatures_[2].temperature_sensor_);
  LOG_SENSOR("", "Temperature 4", this->temperatures_[3].temperature_sensor_);
  LOG_SENSOR("", "Temperature 5", this->temperatures_[4].temperature_sensor_);
  LOG_SENSOR("", "Temperature 6", this->temperatures_[5].temperature_sensor_);
  LOG_SENSOR("", "Cell Voltage 1", this->cells_[0].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 2", this->cells_[1].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 3", this->cells_[2].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 4", this->cells_[3].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 5", this->cells_[4].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 6", this->cells_[5].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 7", this->cells_[6].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 8", this->cells_[7].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 9", this->cells_[8].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 10", this->cells_[9].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 11", this->cells_[10].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 12", this->cells_[11].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 13", this->cells_[12].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 14", this->cells_[13].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 15", this->cells_[14].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 16", this->cells_[15].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 17", this->cells_[16].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 18", this->cells_[17].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 19", this->cells_[18].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 20", this->cells_[19].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 21", this->cells_[20].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 22", this->cells_[21].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 23", this->cells_[22].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 24", this->cells_[23].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 25", this->cells_[24].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 26", this->cells_[25].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 27", this->cells_[26].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 28", this->cells_[27].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 29", this->cells_[28].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 30", this->cells_[29].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 31", this->cells_[30].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 32", this->cells_[31].cell_voltage_sensor_);

  LOG_TEXT_SENSOR("", "Operation status", this->operation_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Errors", this->errors_text_sensor_);
  LOG_TEXT_SENSOR("", "Device model", this->device_model_text_sensor_);
}

void JbdBmsBle::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void JbdBmsBle::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void JbdBmsBle::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

void JbdBmsBle::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

bool JbdBmsBle::change_mosfet_status(uint8_t address, uint8_t bitmask, bool state) {
  if (this->mosfet_status_ == 255) {
    ESP_LOGE(TAG, "Unable to change the Mosfet status because it's unknown");
    return false;
  }

  uint16_t value = (this->mosfet_status_ & (~(1 << bitmask))) | ((uint8_t) state << bitmask);

  this->mosfet_status_ = value;

  value ^= (1 << 0);
  value ^= (1 << 1);

  return this->write_register(address, value);
}

bool JbdBmsBle::write_register(uint8_t address, uint16_t value) {
  uint8_t frame[9];
  uint8_t data_len = 2;

  frame[0] = JBD_PKT_START;
  frame[1] = JBD_CMD_WRITE;
  frame[2] = address;
  frame[3] = data_len;
  frame[4] = value >> 8;
  frame[5] = value >> 0;
  auto crc = chksum_(frame + 2, data_len + 2);
  frame[6] = crc >> 8;
  frame[7] = crc >> 0;
  frame[8] = JBD_PKT_END;

  ESP_LOGV(TAG, "Send command (handle 0x%02X): %s", this->char_command_handle_,
           format_hex_pretty(frame, sizeof(frame)).c_str());
  auto status =
      esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_command_handle_,
                               sizeof(frame), frame, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);
  }

  return (status == 0);
}

bool JbdBmsBle::send_command(uint8_t action, uint8_t function) {
  uint8_t frame[7];
  uint8_t data_len = 0;

  frame[0] = JBD_PKT_START;
  frame[1] = action;
  frame[2] = function;
  frame[3] = data_len;
  auto crc = chksum_(frame + 2, data_len + 2);
  frame[4] = crc >> 8;
  frame[5] = crc >> 0;
  frame[6] = JBD_PKT_END;

  ESP_LOGV(TAG, "Send command (handle 0x%02X): %s", this->char_command_handle_,
           format_hex_pretty(frame, sizeof(frame)).c_str());
  auto status =
      esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(), this->char_command_handle_,
                               sizeof(frame), frame, ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status) {
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);
  }

  return (status == 0);
}

std::string JbdBmsBle::error_bits_to_string_(const uint16_t mask) {
  std::string values = "";
  if (mask) {
    for (int i = 0; i < ERRORS_SIZE; i++) {
      if (mask & (1 << i)) {
        values.append(ERRORS[i]);
        values.append(";");
      }
    }
    if (!values.empty()) {
      values.pop_back();
    }
  }
  return values;
}

}  // namespace jbd_bms_ble
}  // namespace esphome
