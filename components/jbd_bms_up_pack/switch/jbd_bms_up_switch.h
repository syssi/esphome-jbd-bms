#pragma once

#include "../jbd_bms_up_pack.h"
#include "esphome/components/switch/switch.h"

namespace esphome::jbd_bms_up_pack {

class JbdBmsUpSwitch : public switch_::Switch {
 public:
  void set_parent(JbdBmsUpPack *parent) { parent_ = parent; }
  void set_bit(uint8_t bit) { bit_ = bit; }

 protected:
  void write_state(bool state) override;
  JbdBmsUpPack *parent_{nullptr};
  uint8_t bit_{0};
};

}  // namespace esphome::jbd_bms_up_pack
