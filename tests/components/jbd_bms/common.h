#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/jbd_bms/jbd_bms.h"
#include "esphome/components/switch/switch.h"

namespace esphome::jbd_bms::testing {

class TestableJbdBms : public JbdBms {
 public:
  void update() override {}
  void send_command(uint8_t action, uint8_t function) override {}
};

class TestableSwitch : public switch_::Switch {
 public:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace esphome::jbd_bms::testing
