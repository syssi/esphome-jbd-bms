#include "jbd_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace jbd_bms_ble {

static const char *const TAG = "jbd_bms_ble.button";

static const uint8_t JBD_CMD_READ = 0xA5;
static const uint8_t JBD_CMD_FORCE_SOC_RESET = 0x0A;

void JbdButton::dump_config() { LOG_BUTTON("", "JbdBmsBle Button", this); }
void JbdButton::press_action() {
  if (this->holding_register_ == JBD_CMD_FORCE_SOC_RESET) {
    this->parent_->write_register(this->holding_register_, 0x0100);
    return;
  }

  this->parent_->send_command(JBD_CMD_READ, this->holding_register_);
}

}  // namespace jbd_bms_ble
}  // namespace esphome
