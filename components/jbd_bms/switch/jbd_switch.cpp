#include "jbd_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace jbd_bms {

static const char *const TAG = "jbd_bms.switch";

void JbdSwitch::dump_config() { LOG_SWITCH("", "JbdBms Switch", this); }
void JbdSwitch::write_state(bool state) {
  this->parent_->change_mosfet_status(this->holding_register_, this->bitmask_, state);
}

}  // namespace jbd_bms
}  // namespace esphome
