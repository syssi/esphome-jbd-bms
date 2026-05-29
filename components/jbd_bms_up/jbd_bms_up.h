#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/components/uart/uart.h"

namespace esphome::jbd_bms_up {

class JbdBmsUpDevice;

class JbdBmsUp : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void register_device(JbdBmsUpDevice *device) { this->devices_.push_back(device); }
  void set_rx_timeout(uint16_t rx_timeout) { rx_timeout_ = rx_timeout; }
  void set_flow_control_pin(GPIOPin *flow_control_pin) { this->flow_control_pin_ = flow_control_pin; }

  void send(uint8_t address, uint8_t function, uint16_t start_addr, uint16_t end_addr,
            const std::vector<uint8_t> &data = {});

 protected:
  std::vector<uint8_t> build_frame_(uint8_t address, uint8_t function, uint16_t start_addr, uint16_t end_addr,
                                    const std::vector<uint8_t> &data = {});
  bool parse_byte_(uint8_t byte);

  GPIOPin *flow_control_pin_{nullptr};
  std::vector<uint8_t> rx_buffer_;
  uint16_t rx_timeout_{150};
  uint32_t last_byte_{0};
  std::vector<JbdBmsUpDevice *> devices_;
};

uint16_t crc16(const uint8_t *data, size_t len);

class JbdBmsUpDevice {
 public:
  void set_parent(JbdBmsUp *parent) { parent_ = parent; }
  void set_address(uint8_t address) { address_ = address; }
  virtual void on_jbd_bms_up_data(uint8_t function, uint16_t start_addr, const std::vector<uint8_t> &data) = 0;

  virtual void send(uint8_t function, uint16_t start_addr, uint16_t end_addr, const std::vector<uint8_t> &data) {
    this->parent_->send(address_, function, start_addr, end_addr, data);
  }

 protected:
  friend JbdBmsUp;

  JbdBmsUp *parent_{nullptr};
  uint8_t address_{1};
};

}  // namespace esphome::jbd_bms_up
