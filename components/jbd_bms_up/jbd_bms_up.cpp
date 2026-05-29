#include "jbd_bms_up.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome::jbd_bms_up {

static const char *const TAG = "jbd_bms_up";

uint16_t crc16(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      crc = (crc & 0x01) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
  }
  return crc;
}

void JbdBmsUp::setup() {
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
    this->flow_control_pin_->digital_write(false);
  }
}

void JbdBmsUp::loop() {
  const uint32_t now = millis();
  if (now - this->last_byte_ > this->rx_timeout_) {
    if (!this->rx_buffer_.empty()) {
      ESP_LOGVV(TAG, "Buffer cleared due to timeout: %s",
                format_hex_pretty(&this->rx_buffer_.front(), this->rx_buffer_.size()).c_str());  // NOLINT
      this->rx_buffer_.clear();
    }
    this->last_byte_ = now;
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    if (this->parse_byte_(byte)) {
      this->last_byte_ = now;
    } else {
      if (!this->rx_buffer_.empty()) {
        ESP_LOGVV(TAG, "Buffer cleared due to reset: %s",
                  format_hex_pretty(&this->rx_buffer_.front(), this->rx_buffer_.size()).c_str());  // NOLINT
      }
      this->rx_buffer_.clear();
    }
  }
}

bool JbdBmsUp::parse_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];

  // Frame: [Addr 1B][FC 1B][StartAddr 2B][EndAddr 2B][DataLen 2B][Data...][CRC16 2B LSB]

  if (at == 0) {
    return raw[0] >= 1 && raw[0] <= 247;
  }

  if (at < 7) {
    return true;
  }

  uint16_t data_len = (uint16_t(raw[6]) << 8) | raw[7];

  if (data_len > 512) {
    ESP_LOGW(TAG, "Implausible data length: %u", data_len);
    return false;
  }

  uint16_t frame_len = 8 + data_len + 2;

  if (at < frame_len - 1) {
    return true;
  }

  uint16_t computed_crc = crc16(raw, 8 + data_len);
  uint16_t remote_crc = uint16_t(raw[8 + data_len]) | (uint16_t(raw[9 + data_len]) << 8);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, remote_crc);
    return false;
  }

  uint8_t address = raw[0];
  uint8_t function = raw[1];
  uint16_t start_addr = (uint16_t(raw[2]) << 8) | raw[3];

  ESP_LOGV(TAG, "RX <- %s", format_hex_pretty(raw, frame_len).c_str());  // NOLINT

  std::vector<uint8_t> data(this->rx_buffer_.begin() + 8, this->rx_buffer_.begin() + 8 + data_len);

  bool found = false;
  for (auto *device : this->devices_) {
    if (device->address_ == address) {
      device->on_jbd_bms_up_data(function, start_addr, data);
      found = true;
    }
  }
  if (!found) {
    ESP_LOGW(TAG, "Got frame from unknown address 0x%02X!", address);
  }

  return false;
}

std::vector<uint8_t> JbdBmsUp::build_frame_(uint8_t address, uint8_t function, uint16_t start_addr, uint16_t end_addr,
                                            const std::vector<uint8_t> &data) {
  size_t payload_len = data.size();
  std::vector<uint8_t> frame(8 + payload_len + 2);

  frame[0] = address;
  frame[1] = function;
  frame[2] = start_addr >> 8;
  frame[3] = start_addr & 0xFF;
  frame[4] = end_addr >> 8;
  frame[5] = end_addr & 0xFF;
  frame[6] = payload_len >> 8;
  frame[7] = payload_len & 0xFF;
  std::copy(data.begin(), data.end(), frame.begin() + 8);

  uint16_t crc = crc16(frame.data(), 8 + payload_len);
  frame[8 + payload_len] = crc & 0xFF;
  frame[9 + payload_len] = crc >> 8;

  return frame;
}

void JbdBmsUp::send(uint8_t address, uint8_t function, uint16_t start_addr, uint16_t end_addr,
                    const std::vector<uint8_t> &data) {
  auto frame = build_frame_(address, function, start_addr, end_addr, data);

  ESP_LOGV(TAG, "TX -> %s", format_hex_pretty(frame.data(), frame.size()).c_str());  // NOLINT

  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->digital_write(true);
  }
  this->write_array(frame.data(), frame.size());
  this->flush();
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->digital_write(false);
  }
}

void JbdBmsUp::dump_config() {
  ESP_LOGCONFIG(TAG, "JbdBmsUp:");
  ESP_LOGCONFIG(TAG, "  RX timeout: %d ms", this->rx_timeout_);
  LOG_PIN("  Flow Control Pin: ", this->flow_control_pin_);
}

float JbdBmsUp::get_setup_priority() const { return setup_priority::BUS - 1.0f; }

}  // namespace esphome::jbd_bms_up
