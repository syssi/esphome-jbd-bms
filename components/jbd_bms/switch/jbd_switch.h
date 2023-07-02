#pragma once

#include "../jbd_bms.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace jbd_bms {

class JbdBms;
class JbdSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(JbdBms *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t holding_register) { this->holding_register_ = holding_register; };
  void set_bitmask(uint8_t bitmask) { this->bitmask_ = bitmask; };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void write_state(bool state) override;
  JbdBms *parent_;
  uint8_t holding_register_;
  uint8_t bitmask_;
};

}  // namespace jbd_bms
}  // namespace esphome
