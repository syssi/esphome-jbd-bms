#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/jbd_bms_ble/jbd_bms_ble.h"
#include "esphome/components/switch/switch.h"

namespace esphome::jbd_bms_ble::testing {

class TestableJbdBmsBle : public JbdBmsBle {
 public:
  void update() override {}
};

class TestableSwitch : public switch_::Switch {
 public:
  void write_state(bool state) override { this->publish_state(state); }
};

}  // namespace esphome::jbd_bms_ble::testing
