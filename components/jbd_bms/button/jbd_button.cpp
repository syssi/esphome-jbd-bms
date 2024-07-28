#include "jbd_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace jbd_bms {

static const char *const TAG = "jbd_bms.button";

void JbdButton::dump_config() { LOG_BUTTON("", "JbdBms Button", this); }
void JbdButton::press_action() { this->parent_->send_command(JBD_CMD_READ, this->holding_register_); }

}  // namespace jbd_bms
}  // namespace esphome
