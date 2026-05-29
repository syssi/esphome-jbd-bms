#include "jbd_bms_up_switch.h"
#include "esphome/core/log.h"

namespace esphome::jbd_bms_up_pack {

static const char *const TAG = "jbd_bms_up_pack.switch";

void JbdBmsUpSwitch::dump_config() { LOG_SWITCH("", "JbdBmsUpPack Switch", this); }

void JbdBmsUpSwitch::write_state(bool state) {
  if (this->parent_->change_mosfet_status(this->bit_, state)) {
    this->publish_state(state);
  }
}

}  // namespace esphome::jbd_bms_up_pack
