#pragma once

#include "../jbd_bms_ble.h"
#include "esphome/core/component.h"
#include "esphome/components/button/button.h"

namespace esphome {
namespace jbd_bms_ble {

class JbdBmsBle;
class JbdButton : public button::Button, public Component {
 public:
  void set_parent(JbdBmsBle *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t holding_register) { this->holding_register_ = holding_register; };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void press_action() override;
  JbdBmsBle *parent_;
  uint8_t holding_register_;
};

}  // namespace jbd_bms_ble
}  // namespace esphome
