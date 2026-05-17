#include "jbd_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome::jbd_bms_ble {

static const char *const TAG = "jbd_bms_ble.button";

void JbdButton::dump_config() { LOG_BUTTON("", "JbdBmsBle Button", this); }
void JbdButton::press_action() {
  this->parent_->send_command(this->command_, this->address_, this->payload_, this->length_);
}

}  // namespace esphome::jbd_bms_ble
