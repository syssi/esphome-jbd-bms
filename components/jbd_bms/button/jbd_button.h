#pragma once

#include "../jbd_bms.h"
#include "esphome/core/component.h"
#include "esphome/components/button/button.h"

namespace esphome::jbd_bms {

class JbdBms;
class JbdButton : public button::Button, public Component {
 public:
  void set_parent(JbdBms *parent) { this->parent_ = parent; };
  void set_command(uint8_t command) { this->command_ = command; };
  void set_address(uint8_t address) { this->address_ = address; };
  void set_payload(uint8_t high, uint8_t low) {
    this->payload_[0] = high;
    this->payload_[1] = low;
    this->length_ = 2;
  };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void press_action() override;
  JbdBms *parent_;
  uint8_t command_{0};
  uint8_t address_{0};
  uint8_t payload_[2]{};
  uint8_t length_{0};
};

}  // namespace esphome::jbd_bms
