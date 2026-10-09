#include "jbd_bms_up_switch.h"
#include "esphome/core/log.h"

// Fallback for ESPHome < 2026.10.0
#ifndef ESPHOME_LOG_TAG
#define ESPHOME_LOG_TAG(name, tag) static const char *const name = tag
#endif

namespace esphome::jbd_bms_up_pack {

ESPHOME_LOG_TAG(TAG, "jbd_bms_up_pack.switch");

void JbdBmsUpSwitch::dump_config() { LOG_SWITCH("", "JbdBmsUpPack Switch", this); }

void JbdBmsUpSwitch::write_state(bool state) {
  if (this->parent_->change_mosfet_status(this->bit_, state)) {
    this->publish_state(state);
  }
}

}  // namespace esphome::jbd_bms_up_pack
