#include "jbd_bms_up_pack.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome::jbd_bms_up_pack {

static const char *const TAG = "jbd_bms_up_pack";

static const uint8_t JBD_FC_READ = 0x78;
static const uint8_t JBD_FC_WRITE = 0x79;

static const uint16_t ADDR_PACK_STATUS = 0x1000;
static const uint16_t ADDR_PACK_STATUS_END = 0x10A0;

static const uint16_t ADDR_MOSFET_CTRL = 0x2902;
static const uint16_t ADDR_MOSFET_CTRL_END = 0x2904;

static const uint8_t MOSFET_BIT_DSG = 0;
static const uint8_t MOSFET_BIT_CHG = 1;

static const uint8_t ERRORS_SIZE = 18;
static const char *const ERRORS[ERRORS_SIZE] = {
    "Cell overvoltage",                  // 0x000001
    "Cell undervoltage",                 // 0x000002
    "Pack overvoltage",                  // 0x000004
    "Pack undervoltage",                 // 0x000008
    "Charging overcurrent",              // 0x000010
    "Discharging overcurrent",           // 0x000020
    "Charging over temperature",         // 0x000040
    "Charging under temperature",        // 0x000080
    "Discharging over temperature",      // 0x000100
    "Discharging under temperature",     // 0x000200
    "Mosfet over temperature",           // 0x000400
    "Ambient over temperature",          // 0x000800
    "Ambient under temperature",         // 0x001000
    "Voltage difference too large",      // 0x002000
    "Temperature difference too large",  // 0x004000
    "SOC too low",                       // 0x008000
    "EEP fault",                         // 0x010000
    "Real time clock abnormal",          // 0x020000
};

static const uint8_t PROTECT_SIZE = 27;
static const char *const PROTECT[PROTECT_SIZE] = {
    "Cell overvoltage",                  // 0x0000001
    "Cell undervoltage",                 // 0x0000002
    "Pack overvoltage",                  // 0x0000004
    "Pack undervoltage",                 // 0x0000008
    "Charging overcurrent 1",            // 0x0000010
    "Charging overcurrent 2",            // 0x0000020
    "Discharging overcurrent 1",         // 0x0000040
    "Discharging overcurrent 2",         // 0x0000080
    "Charging over temperature",         // 0x0000100
    "Charging under temperature",        // 0x0000200
    "Discharging over temperature",      // 0x0000400
    "Discharging under temperature",     // 0x0000800
    "Mosfet over temperature",           // 0x0001000
    "Ambient over temperature",          // 0x0002000
    "Ambient under temperature",         // 0x0004000
    "Voltage difference too large",      // 0x0008000
    "Temperature difference too large",  // 0x0010000
    "SOC too low",                       // 0x0020000
    "Short circuit",                     // 0x0040000
    "Monomer offline",                   // 0x0080000
    "Temperature drop",                  // 0x0100000
    "Charge Mosfet fault",               // 0x0200000
    "Discharge Mosfet fault",            // 0x0400000
    "Current limiting fault",            // 0x0800000
    "Aerosol fault",                     // 0x1000000
    "Full charge protection",            // 0x2000000
    "Abnormal AFE communication",        // 0x4000000
};

static const uint8_t OPERATION_STATUS_SIZE = 3;
static const char *const OPERATION_STATUS[OPERATION_STATUS_SIZE] = {
    "Idle",         // 0x00
    "Charging",     // 0x01
    "Discharging",  // 0x02
};

void JbdBmsUpPack::setup() { ESP_LOGCONFIG(TAG, "Setting up JbdBmsUpPack 0x%02X", this->address_); }

void JbdBmsUpPack::update() { this->send(JBD_FC_READ, ADDR_PACK_STATUS, ADDR_PACK_STATUS_END, {}); }

void JbdBmsUpPack::on_jbd_bms_up_data(uint8_t function, uint16_t start_addr, const std::vector<uint8_t> &data) {
  if (function == JBD_FC_READ && start_addr == ADDR_PACK_STATUS) {
    this->on_pack_status_(data);
    return;
  }
  if (function == JBD_FC_WRITE && start_addr == ADDR_MOSFET_CTRL) {
    ESP_LOGD(TAG, "MOSFET write acknowledged");
    return;
  }
  ESP_LOGW(TAG, "Unhandled response (function 0x%02X, addr 0x%04X): %s", function, start_addr,
           format_hex_pretty(&data.front(), data.size()).c_str());  // NOLINT
}

void JbdBmsUpPack::on_pack_status_(const std::vector<uint8_t> &data) {
  if (data.size() < 70) {
    ESP_LOGW(TAG, "Pack status frame too short: %zu bytes", data.size());
    return;
  }

  auto jbd_get_16bit = [&](size_t i) -> uint16_t { return (uint16_t(data[i]) << 8) | data[i + 1]; };
  auto jbd_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(data[i]) << 24) | (uint32_t(data[i + 1]) << 16) | (uint32_t(data[i + 2]) << 8) | data[i + 3];
  };

  ESP_LOGD(TAG, "Pack status frame (%zu bytes) received", data.size());

  float total_voltage = jbd_get_16bit(0) * 0.01f;
  float current = (jbd_get_32bit(4) - 300000.0f) * 0.01f;
  float power = total_voltage * current;

  this->publish_state_(total_voltage_sensor_, total_voltage);
  this->publish_state_(current_sensor_, current);
  this->publish_state_(power_sensor_, power);
  this->publish_state_(charging_power_sensor_, std::max(0.0f, power));
  this->publish_state_(discharging_power_sensor_, std::abs(std::min(0.0f, power)));

  this->publish_state_(state_of_charge_sensor_, jbd_get_16bit(8) * 0.01f);
  this->publish_state_(capacity_remaining_sensor_, jbd_get_16bit(10) * 0.01f);
  this->publish_state_(nominal_capacity_sensor_, jbd_get_16bit(12) * 0.01f);
  this->publish_state_(rated_capacity_sensor_, jbd_get_16bit(14) * 0.01f);

  this->publish_state_(mosfet_temperature_sensor_, (jbd_get_16bit(16) - 500) * 0.1f);
  this->publish_state_(ambient_temperature_sensor_, (jbd_get_16bit(18) - 500) * 0.1f);

  uint16_t op_status = jbd_get_16bit(20);
  this->publish_state_(operation_status_bitmask_sensor_, op_status);
  if (op_status < OPERATION_STATUS_SIZE) {
    this->publish_state_(operation_status_text_sensor_, OPERATION_STATUS[op_status]);
  } else {
    this->publish_state_(operation_status_text_sensor_, "Unknown");
  }

  this->publish_state_(state_of_health_sensor_, (float) jbd_get_16bit(22));

  uint32_t protect_bitmask = (uint32_t(jbd_get_16bit(24)) << 16) | jbd_get_16bit(26);
  uint32_t errors_bitmask = (uint32_t(jbd_get_16bit(28)) << 16) | jbd_get_16bit(30);
  this->publish_state_(protect_bitmask_sensor_, (float) protect_bitmask);
  this->publish_state_(protect_text_sensor_, bitmask_to_string_(PROTECT, PROTECT_SIZE, protect_bitmask));
  this->publish_state_(errors_bitmask_sensor_, (float) errors_bitmask);
  this->publish_state_(errors_text_sensor_, bitmask_to_string_(ERRORS, ERRORS_SIZE, errors_bitmask));

  uint16_t mos = jbd_get_16bit(32);
  this->mosfet_status_ = mos;
  this->publish_state_(discharging_binary_sensor_, mos & 0x01);
  this->publish_state_(charging_binary_sensor_, mos & 0x02);
  this->publish_state_(precharging_binary_sensor_, mos & 0x04);
  this->publish_state_(heat_binary_sensor_, mos & 0x08);
  this->publish_state_(fan_binary_sensor_, mos & 0x10);

  this->publish_state_(charging_cycles_sensor_, (float) jbd_get_16bit(36));

  // Byte layout verified against UP16S020 raw capture (FC=0x78, reg=0x1000):
  // [38-39] max_voltage_cell index (1-based), [40-41] max_cell_voltage mV
  // [42-43] min_voltage_cell index, [44-45] min_cell_voltage mV
  // [46-47] average_cell_voltage mV, [48-49] max_temp_sensor index
  // [50-51] max_temperature, [52-53] min_temp_sensor index, [54-55] min_temperature
  // [56-57] average_temperature, [58-59] CVL V/10, [60-61] CCL A/10
  // [62-63] DVL V/10, [64-65] DCL A/10, [66-67] cell count, [68+] cell voltage array
  this->publish_state_(max_voltage_cell_sensor_, (float) jbd_get_16bit(38));
  float max_cell_v = jbd_get_16bit(40) * 0.001f;
  this->publish_state_(max_cell_voltage_sensor_, max_cell_v);
  this->publish_state_(min_voltage_cell_sensor_, (float) jbd_get_16bit(42));
  float min_cell_v = jbd_get_16bit(44) * 0.001f;
  this->publish_state_(min_cell_voltage_sensor_, min_cell_v);
  this->publish_state_(average_cell_voltage_sensor_, jbd_get_16bit(46) * 0.001f);
  this->publish_state_(delta_cell_voltage_sensor_, max_cell_v - min_cell_v);

  this->publish_state_(max_temperature_sensor_, (jbd_get_16bit(50) - 500) * 0.1f);
  this->publish_state_(min_temperature_sensor_, (jbd_get_16bit(54) - 500) * 0.1f);
  this->publish_state_(average_temperature_sensor_, (jbd_get_16bit(56) - 500) * 0.1f);
  this->publish_state_(charge_voltage_limit_sensor_, jbd_get_16bit(58) * 0.1f);
  this->publish_state_(charge_current_limit_sensor_, jbd_get_16bit(60) * 0.1f);
  this->publish_state_(discharge_voltage_limit_sensor_, jbd_get_16bit(62) * 0.1f);
  this->publish_state_(discharge_current_limit_sensor_, jbd_get_16bit(64) * 0.1f);

  if (data.size() < 68) {
    return;
  }

  uint16_t cell_count = jbd_get_16bit(66);
  this->publish_state_(battery_strings_sensor_, (float) cell_count);

  size_t offset = 68;
  uint8_t cell_sensor_count = std::min((uint16_t) 32, cell_count);
  if (data.size() < offset + 2 * cell_sensor_count + 2) {
    return;
  }

  for (uint8_t i = 0; i < cell_sensor_count; i++) {
    this->publish_state_(cell_voltage_sensors_[i], jbd_get_16bit(offset + 2 * i) * 0.001f);
  }
  offset = offset + 2 * cell_count;

  uint16_t temperature_sensor_count = jbd_get_16bit(offset);
  offset = offset + 2;
  uint8_t temperature_sensor_publish_count = std::min((uint16_t) 6, temperature_sensor_count);
  if (data.size() < offset + 2 * temperature_sensor_publish_count) {
    return;
  }

  for (uint8_t i = 0; i < temperature_sensor_publish_count; i++) {
    this->publish_state_(temperature_sensors_[i], (jbd_get_16bit(offset + 2 * i) - 500) * 0.1f);
  }
  offset = offset + 2 * temperature_sensor_count;

  if (data.size() >= offset + 6) {
    uint16_t balance_status = jbd_get_16bit(offset + 2);
    this->publish_state_(balancing_bitmask_sensor_, (float) balance_status);
    this->publish_state_(balancing_binary_sensor_, balance_status != 0);
    uint16_t fw = jbd_get_16bit(offset + 4);
    char fw_str[8];
    snprintf(fw_str, sizeof(fw_str), "%d.%d", fw >> 8, fw & 0xFF);
    this->publish_state_(firmware_version_text_sensor_, std::string(fw_str));
  }
  offset += 6;

  if (data.size() >= offset + 30) {
    std::string model(data.begin() + offset, data.begin() + offset + 30);
    auto null_pos = model.find('\0');
    if (null_pos != std::string::npos) {
      model.resize(null_pos);
    }
    this->publish_state_(device_model_text_sensor_, model);
  }

  this->publish_state_(online_status_binary_sensor_, true);
}

std::string JbdBmsUpPack::bitmask_to_string_(const char *const messages[], uint8_t messages_size, uint32_t mask) {
  std::string result;
  for (uint8_t i = 0; i < messages_size; i++) {
    if (mask & (1UL << i)) {
      if (!result.empty()) {
        result += ", ";
      }
      result += messages[i];
    }
  }
  return result;
}

void JbdBmsUpPack::dump_config() {
  ESP_LOGCONFIG(TAG, "JbdBmsUpPack:");
  ESP_LOGCONFIG(TAG, "  Address: %d", this->address_);
  ESP_LOGCONFIG(TAG, "  Update interval: %u ms", this->get_update_interval());
  LOG_SENSOR("", "Total Voltage", this->total_voltage_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Charging Power", this->charging_power_sensor_);
  LOG_SENSOR("", "Discharging Power", this->discharging_power_sensor_);
  LOG_SENSOR("", "State of Charge", this->state_of_charge_sensor_);
  LOG_SENSOR("", "Capacity Remaining", this->capacity_remaining_sensor_);
  LOG_SENSOR("", "Nominal Capacity", this->nominal_capacity_sensor_);
  LOG_SENSOR("", "Rated Capacity", this->rated_capacity_sensor_);
  LOG_SENSOR("", "Mosfet Temperature", this->mosfet_temperature_sensor_);
  LOG_SENSOR("", "Ambient Temperature", this->ambient_temperature_sensor_);
  LOG_SENSOR("", "Max Temperature", this->max_temperature_sensor_);
  LOG_SENSOR("", "Min Temperature", this->min_temperature_sensor_);
  LOG_SENSOR("", "Average Temperature", this->average_temperature_sensor_);
  LOG_SENSOR("", "State of Health", this->state_of_health_sensor_);
  LOG_SENSOR("", "Operation Status Bitmask", this->operation_status_bitmask_sensor_);
  LOG_SENSOR("", "Errors Bitmask", this->errors_bitmask_sensor_);
  LOG_SENSOR("", "Protect Bitmask", this->protect_bitmask_sensor_);
  LOG_SENSOR("", "Charging Cycles", this->charging_cycles_sensor_);
  LOG_SENSOR("", "Min Cell Voltage", this->min_cell_voltage_sensor_);
  LOG_SENSOR("", "Max Cell Voltage", this->max_cell_voltage_sensor_);
  LOG_SENSOR("", "Min Voltage Cell", this->min_voltage_cell_sensor_);
  LOG_SENSOR("", "Max Voltage Cell", this->max_voltage_cell_sensor_);
  LOG_SENSOR("", "Delta Cell Voltage", this->delta_cell_voltage_sensor_);
  LOG_SENSOR("", "Average Cell Voltage", this->average_cell_voltage_sensor_);
  LOG_SENSOR("", "Battery Strings", this->battery_strings_sensor_);
  LOG_SENSOR("", "Charge Voltage Limit", this->charge_voltage_limit_sensor_);
  LOG_SENSOR("", "Charge Current Limit", this->charge_current_limit_sensor_);
  LOG_SENSOR("", "Discharge Voltage Limit", this->discharge_voltage_limit_sensor_);
  LOG_SENSOR("", "Discharge Current Limit", this->discharge_current_limit_sensor_);
  LOG_SENSOR("", "Balancing Bitmask", this->balancing_bitmask_sensor_);
  LOG_BINARY_SENSOR("", "Balancing", this->balancing_binary_sensor_);
  LOG_BINARY_SENSOR("", "Charging", this->charging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Discharging", this->discharging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Precharging", this->precharging_binary_sensor_);
  LOG_BINARY_SENSOR("", "Heat", this->heat_binary_sensor_);
  LOG_BINARY_SENSOR("", "Fan", this->fan_binary_sensor_);
  LOG_BINARY_SENSOR("", "Online Status", this->online_status_binary_sensor_);
  LOG_TEXT_SENSOR("", "Firmware Version", this->firmware_version_text_sensor_);
  LOG_TEXT_SENSOR("", "Operation Status", this->operation_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Errors", this->errors_text_sensor_);
  LOG_TEXT_SENSOR("", "Protect", this->protect_text_sensor_);
  LOG_TEXT_SENSOR("", "Device Model", this->device_model_text_sensor_);
  LOG_SWITCH("", "Charging", this->charging_switch_);
  LOG_SWITCH("", "Discharging", this->discharging_switch_);
  LOG_SENSOR("", "Temperature 1", this->temperature_sensors_[0]);
  LOG_SENSOR("", "Temperature 2", this->temperature_sensors_[1]);
  LOG_SENSOR("", "Temperature 3", this->temperature_sensors_[2]);
  LOG_SENSOR("", "Temperature 4", this->temperature_sensors_[3]);
  LOG_SENSOR("", "Temperature 5", this->temperature_sensors_[4]);
  LOG_SENSOR("", "Temperature 6", this->temperature_sensors_[5]);
  LOG_SENSOR("", "Cell Voltage 1", this->cell_voltage_sensors_[0]);
  LOG_SENSOR("", "Cell Voltage 2", this->cell_voltage_sensors_[1]);
  LOG_SENSOR("", "Cell Voltage 3", this->cell_voltage_sensors_[2]);
  LOG_SENSOR("", "Cell Voltage 4", this->cell_voltage_sensors_[3]);
  LOG_SENSOR("", "Cell Voltage 5", this->cell_voltage_sensors_[4]);
  LOG_SENSOR("", "Cell Voltage 6", this->cell_voltage_sensors_[5]);
  LOG_SENSOR("", "Cell Voltage 7", this->cell_voltage_sensors_[6]);
  LOG_SENSOR("", "Cell Voltage 8", this->cell_voltage_sensors_[7]);
  LOG_SENSOR("", "Cell Voltage 9", this->cell_voltage_sensors_[8]);
  LOG_SENSOR("", "Cell Voltage 10", this->cell_voltage_sensors_[9]);
  LOG_SENSOR("", "Cell Voltage 11", this->cell_voltage_sensors_[10]);
  LOG_SENSOR("", "Cell Voltage 12", this->cell_voltage_sensors_[11]);
  LOG_SENSOR("", "Cell Voltage 13", this->cell_voltage_sensors_[12]);
  LOG_SENSOR("", "Cell Voltage 14", this->cell_voltage_sensors_[13]);
  LOG_SENSOR("", "Cell Voltage 15", this->cell_voltage_sensors_[14]);
  LOG_SENSOR("", "Cell Voltage 16", this->cell_voltage_sensors_[15]);
  LOG_SENSOR("", "Cell Voltage 17", this->cell_voltage_sensors_[16]);
  LOG_SENSOR("", "Cell Voltage 18", this->cell_voltage_sensors_[17]);
  LOG_SENSOR("", "Cell Voltage 19", this->cell_voltage_sensors_[18]);
  LOG_SENSOR("", "Cell Voltage 20", this->cell_voltage_sensors_[19]);
  LOG_SENSOR("", "Cell Voltage 21", this->cell_voltage_sensors_[20]);
  LOG_SENSOR("", "Cell Voltage 22", this->cell_voltage_sensors_[21]);
  LOG_SENSOR("", "Cell Voltage 23", this->cell_voltage_sensors_[22]);
  LOG_SENSOR("", "Cell Voltage 24", this->cell_voltage_sensors_[23]);
  LOG_SENSOR("", "Cell Voltage 25", this->cell_voltage_sensors_[24]);
  LOG_SENSOR("", "Cell Voltage 26", this->cell_voltage_sensors_[25]);
  LOG_SENSOR("", "Cell Voltage 27", this->cell_voltage_sensors_[26]);
  LOG_SENSOR("", "Cell Voltage 28", this->cell_voltage_sensors_[27]);
  LOG_SENSOR("", "Cell Voltage 29", this->cell_voltage_sensors_[28]);
  LOG_SENSOR("", "Cell Voltage 30", this->cell_voltage_sensors_[29]);
  LOG_SENSOR("", "Cell Voltage 31", this->cell_voltage_sensors_[30]);
  LOG_SENSOR("", "Cell Voltage 32", this->cell_voltage_sensors_[31]);
}

bool JbdBmsUpPack::change_mosfet_status(uint8_t bit, bool state) {
  if (this->mosfet_status_ == 0xFFFF) {
    ESP_LOGE(TAG, "MOSFET status unknown, cannot change");
    return false;
  }
  uint16_t new_status = state ? (this->mosfet_status_ | (1u << bit)) : (this->mosfet_status_ & ~(1u << bit));
  std::vector<uint8_t> payload = {0x11, 0x4A, 0x42, 0x44, (uint8_t) (new_status >> 8), (uint8_t) (new_status & 0xFF)};
  this->send(JBD_FC_WRITE, ADDR_MOSFET_CTRL, ADDR_MOSFET_CTRL_END, payload);
  this->mosfet_status_ = new_status;
  return true;
}

void JbdBmsUpPack::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor != nullptr) {
    sensor->publish_state(value);
  }
}

void JbdBmsUpPack::publish_state_(binary_sensor::BinarySensor *sensor, bool value) {
  if (sensor != nullptr) {
    sensor->publish_state(value);
  }
}

void JbdBmsUpPack::publish_state_(text_sensor::TextSensor *sensor, const std::string &value) {
  if (sensor != nullptr) {
    sensor->publish_state(value);
  }
}

}  // namespace esphome::jbd_bms_up_pack
