#pragma once

#include "../jbd_bms_ble.h"
#include "esphome/components/switch/switch.h"

namespace esphome::jbd_bms_ble {

class JbdBmsBle;
class JbdSwitch : public switch_::Switch {
 public:
  void set_parent(JbdBmsBle *parent) { this->parent_ = parent; };
  void set_address(uint8_t address) { this->address_ = address; };
  void set_bitmask(uint8_t bitmask) { this->bitmask_ = bitmask; };

 protected:
  void write_state(bool state) override;
  JbdBmsBle *parent_;
  uint8_t address_;
  uint8_t bitmask_;
};

}  // namespace esphome::jbd_bms_ble
