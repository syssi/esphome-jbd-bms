#include "jbd_bms_up.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "jbd_bms_commands.h"

namespace esphome {
namespace jbd_bms {

static const char *const TAG = "jbd_bms_up";

enum JBDAddress {
    JBD_PACK_STATUS = 0x1000,
};

enum JBDFunction {
    JBD_READ_BMS = 0x78,
    JBD_WRITE_BMS = 0x79,
};

bool JbdBmsUP::parse_jbd_bms_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];

  // Example response
  // 0x01 : Bank
  // 0x78 : Function code
  // 0x1000 : Start address
  // 0x10a0 : End address
  // 0x0000 : Data length
  // <data> : data
  // 0x7fb2 : crc16 (LSB 1st)

  if (at == 0) {
    if (raw[0] > 16) {
      ESP_LOGW(TAG, "Invalid header: 0x%02X", raw[0]);

      // return false to reset buffer
      return false;
    }

    return true;
  }

  // Byte 1 (Function)
  // Byte 2,3 (Start addresss)
  // Byte 4,5 (End addresss)
  // Byte 6,7 (Length)
  if (at < 8)
    return true;

  uint16_t data_len = (raw[6] << 8) | raw[7];
  uint16_t frame_len = 8 + data_len + 2;

  if (at < frame_len - 1)
    return true;

  uint8_t bat_address = raw[0];
  uint8_t function = raw[1];
  uint16_t computed_crc = crc16(raw, data_len + 8);
  uint16_t remote_crc = (uint16_t(raw[8 + data_len]) << 0) | (uint16_t(raw[9 + data_len]) << 8);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, remote_crc);
    return false;
  }
  uint16_t address = (uint16_t(raw[2]) << 8) | (uint16_t(raw[3]) << 0)
  ESP_LOGVV(TAG, "RX <- %s", format_hex_pretty(raw, at).c_str());


  std::vector<uint8_t> data(this->rx_buffer_.begin() + 8, this->rx_buffer_.begin() + frame_len - 2);
  bool found = false;
  for (auto *battery : this->batteries_) {
    if (battery->address() != bat_address) {
      continue;
    }
    found = true;
    battery->on_jbd_bms_data(function, address, data);
    break;
  }
  if (!found) {
    ESP_LOGW(TAG, "Got unknown battery address 0x%02X! ", bat_address);
  }

  // reset buffer
  ESP_LOGV(TAG, "Clearing buffer of %d bytes - parse succeeded", at);
  this->rx_buffer_.clear();
  return true;
}
    
void JbdBmsUP::on_jbd_bms_data(const uint8_t &function, const uint16_t &reg_address, const std::vector<uint8_t> &data)
{
    this->reset_online_status_tracker_();
    if (function == JBD_READ_BMS) {
        switch (reg_address) {
            case JBD_PACK_STATUS:
                ESP_LOGVV(TAG, "Got pack status");
                on_pack_status(data);
                return;
        }
    }
    ESP_LOGW(TAG, "Unhandled response (function 0x%02X) received: %s", function,
             format_hex_pretty(&data.front(), data.size()).c_str());
}

void JbdBmsUP::on_pack_status(const std::vector<uint8_t> &data) {
    auto get_16bit = [&](size_t i) -> uint16_t {
        return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
    };
    auto get_32bit = [&](size_t i) -> uint32_t {
        return (uint32_t(data[i + 0]) << 24) | (uint32_t(data[i + 1]) << 16) | (uint32_t(data[i + 2]) << 8) | (uint32_t(data[i + 3]) << 0);
    };

    float total_voltage = (float)get_16bit(0) * 0.01;
    float current = ((float)get_32bit(4) - 300000.0) * 0.01;
    float power = total_voltage * current;
    this->publish_state_(this->total_voltage_sensor_, total_voltage);
    this->publish_state_(this->current_sensor_, current);
    this->publish_state_(this->power_sensor_, power);
    this->publish_state_(this->charging_power_sensor_, std::max(0.0f, power));               // 500W vs 0W -> 500W
    this->publish_state_(this->discharging_power_sensor_, std::abs(std::min(0.0f, power)));  // -500W vs 0W -> 500W

    this->publish_state_(this->state_of_charge_sensor_, (float)get_16bit(8) * 0.01);
    this->publish_state_(this->capacity_remaining_sensor_, (float)get_16bit(10) * 0.01);
    this->publish_state_(this->nominal_capacity_sensor_, (float)get_16bit(12) * 0.01);
    this->publish_state_(this->mosfet_temperature_sensor_, ((float)get_16bit(16) - 500) * 0.1);
    this->publish_state_(this->ambient_temperature_sensor_, ((float)get_16bit(18) - 500) * 0.1);
    this->publish_state_(this->state_of_health_sensor_, (float)get_16bit(22));

    this->mosfet_status_ = get_16bit(32);
    auto mos_states = this->mosfet_status_;
    this->publish_state_(discharging_binary_sensor_, mos_states & 0x01);
    this->publish_state_(charging_binary_sensor_, mos_states & 0x02);
    this->publish_state_(precharging_binary_sensor_, mos_states & 0x04);
    this->publish_state_(heat_binary_sensor_, mos_states & 0x08);
    this->publish_state_(fan_binary_sensor_, mos_states & 0x10);

    this->publish_state_(this->charging_cycles_sensor_, get_16bit(36));

    this->publish_state_(this->max_voltage_cell_sensor_, get_16bit(38));
    float max_cell_voltage = (float)get_16bit(40) * 0.001;
    this->publish_state_(this->max_cell_voltage_sensor_, max_cell_voltage);
    this->publish_state_(this->min_voltage_cell_sensor_, get_16bit(42));
    float min_cell_voltage = (float)get_16bit(44) * 0.001;
    this->publish_state_(this->min_cell_voltage_sensor_, min_cell_voltage);
    this->publish_state_(this->average_cell_voltage_sensor_, (float)get_16bit(46) * 0.001);
    this->publish_state_(this->delta_cell_voltage_sensor_, max_cell_voltage - min_cell_voltage);
    int num_cells = get_16bit(66);
    this->publish_state_(this->battery_strings_sensor_, num_cells);
    for (int i = 0; i < std::min(num_cells, 32) ; i++) {
        this->publish_state_(this->cells_[i].cell_voltage_sensor_, (float)get_16bit(68+2*i) * 0.001);
    }
    int pos = 68 + 2 * num_cells;
    int num_temperature_sensors = get_16bit(pos);
    pos += 2;
    for (int i = 0; i < std::min(num_temperature_sensors, 6) ; i++) {
        this->publish_state_(this->temperatures_[i].temperature_sensor_,
                             (float)((uint32_t)get_16bit(pos+2*i) - 500) * 0.001);
    }
    pos += 2*num_temperature_sensors;
    pos += 6;
    this->publish_state_(device_model_text_sensor_, std::string(data.begin() + pos, data.begin() + pos + 30));
}

bool JbdBmsUP::change_mosfet_status(uint8_t address, uint8_t bitmask, bool state) {
    if (this->mosfet_status_ == 255) {
        ESP_LOGE(TAG, "Unable to change the Mosfet status because it's unknown");
        return false;
    }

    uint16_t value = (this->mosfet_status_ & (~(1 << bitmask))) | ((uint8_t) state << bitmask);

    std::vector<uint8_t> data(6);
    data[0] = 0x11;
    data[1] = 'J';
    data[2] = 'B';
    data[3] = 'D';
    data[4] = value >> 8;
    data[5] = value & 0xff;
    send_command(JBD_WRITE_BMS, 0x2902, 0x2904, data);
    return true;
}

void JbdBmsUP::send_command(uint8_t action, uint8_t function)
{
    std::vector<uint8_t>data;
    if (action == JBD_CMD_READ) {
        if (function == JBD_CMD_HWINFO) {
            send_command(0x78, 0x1000, 0x10a0, data);
            return;
        }
    }
    ESP_LOGW(TAG, "Invallid command: %02x, %02x", action, function);

}

void JbdBmsUP::send_command(uint8_t function, uint16_t start_address, uint16_t end_address, const std::vector<uint8_t> &data) {
    uint8_t header[256];
    uint8_t crc_arr[2];
    if (data.size() > 246) {
        ESP_LOGE(TAG, "Data length %d > 246.  Cannot send", data.size());
        return;
    }
    header[0] = address_;
    header[1] = function;
    header[2] = start_address >> 8;
    header[3] = start_address & 0xff;
    header[4] = end_address >> 8;
    header[5] = end_address & 0xff;
    header[6] = data.size() >> 8;
    header[7] = data.size() & 0xff;
    if (data.size()) {
        memcpy(header+8, data.data(), data.size());
    }
    auto crc = crc16(header, data.size() + 8);
    header[8 + data.size()] = crc & 0xff;
    header[9 + data.size()] = crc >> 8;
    ESP_LOGVV(TAG, "Send command: %s",
              format_hex_pretty(header, 10 + data.size()).c_str());
    this->write_array(header, 10 + data.size());
    this->flush();
}

}  // namespace jbd_bms
}  // namespace esphome
