#pragma once

#include "../jbd_bms_up_pack.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome::jbd_bms_up_pack {

class JbdBmsUpSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(JbdBmsUpPack *parent) { parent_ = parent; }
  void set_bit(uint8_t bit) { bit_ = bit; }
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void write_state(bool state) override;
  JbdBmsUpPack *parent_{nullptr};
  uint8_t bit_{0};
};

}  // namespace esphome::jbd_bms_up_pack
