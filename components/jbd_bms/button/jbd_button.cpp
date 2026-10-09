#include "jbd_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

// Fallback for ESPHome < 2026.10.0
#ifndef ESPHOME_LOG_TAG
#define ESPHOME_LOG_TAG(name, tag) static const char *const name = tag
#endif

namespace esphome::jbd_bms {

ESPHOME_LOG_TAG(TAG, "jbd_bms.button");

void JbdButton::dump_config() { LOG_BUTTON("", "JbdBms Button", this); }
void JbdButton::press_action() {
  this->parent_->send_command(this->command_, this->address_, this->payload_, this->length_);
}

}  // namespace esphome::jbd_bms
