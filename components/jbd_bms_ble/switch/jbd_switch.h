#pragma once

#include "../jbd_bms_ble.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome::jbd_bms_ble {

class JbdBmsBle;
class JbdSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(JbdBmsBle *parent) { this->parent_ = parent; };
  void set_address(uint8_t address) { this->address_ = address; };
  void set_bitmask(uint8_t bitmask) { this->bitmask_ = bitmask; };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void write_state(bool state) override;
  JbdBmsBle *parent_;
  uint8_t address_;
  uint8_t bitmask_;
};

}  // namespace esphome::jbd_bms_ble
