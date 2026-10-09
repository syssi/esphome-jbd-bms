#include "jbd_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

// Fallback for ESPHome < 2026.10.0
#ifndef ESPHOME_LOG_TAG
#define ESPHOME_LOG_TAG(name, tag) static const char *const name = tag
#endif

namespace esphome::jbd_bms {

ESPHOME_LOG_TAG(TAG, "jbd_bms.switch");

void JbdSwitch::dump_config() { LOG_SWITCH("", "JbdBms Switch", this); }
void JbdSwitch::write_state(bool state) {
  if (this->parent_->change_mosfet_status(this->address_, this->bitmask_, state)) {
    this->publish_state(state);
  }

  // 0xDD 0x5A 0x01 0x02 0x00 0x00 0xFF 0xFD 0x77
  // this->parent_->write_register(0x01, 0x0000);
}

}  // namespace esphome::jbd_bms
