#include "jbd_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace jbd_bms_ble {

static const char *const TAG = "jbd_bms_ble.button";

static const uint8_t JBD_CMD_READ = 0xA5;

void JbdButton::dump_config() { LOG_BUTTON("", "JbdBmsBle Button", this); }
void JbdButton::press_action() { this->parent_->send_command(JBD_CMD_READ, this->holding_register_); }

}  // namespace jbd_bms_ble
}  // namespace esphome
