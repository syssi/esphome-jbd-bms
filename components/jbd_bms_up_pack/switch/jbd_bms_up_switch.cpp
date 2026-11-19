#include "jbd_bms_up_switch.h"

namespace esphome::jbd_bms_up_pack {

void JbdBmsUpSwitch::write_state(bool state) {
  if (this->parent_->change_mosfet_status(this->bit_, state)) {
    this->publish_state(state);
  }
}

}  // namespace esphome::jbd_bms_up_pack
