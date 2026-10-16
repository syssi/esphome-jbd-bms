#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "esphome/components/jbd_bms_ble/jbd_bms_ble.h"
#include "esphome/components/switch/switch.h"

namespace esphome::jbd_bms_ble::testing {

class TestableJbdBmsBle : public JbdBmsBle {
 public:
  void update() override {}
  bool send_command(uint8_t command, uint8_t address, const uint8_t *data, uint8_t data_len) override {
    last_command_type = command;
    last_command_address = address;
    return true;
  }
  void set_mosfet_status(uint8_t status) { this->mosfet_status_ = status; }
  bool write_register(uint8_t address, uint16_t value) override {
    last_write_address = address;
    last_write_value = value;
    return true;
  }
  using JbdBmsBle::send_command;
  using JbdBmsBle::build_frame_;
  uint8_t last_write_address{0};
  uint16_t last_write_value{0};
  uint8_t last_command_type{0};
  uint8_t last_command_address{0};
};

class TestableSwitch : public switch_::Switch {
 public:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace esphome::jbd_bms_ble::testing
