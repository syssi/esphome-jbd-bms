#include "jbd_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace jbd_bms_ble {

static const char *const TAG = "jbd_bms_ble.switch";

void JbdSwitch::dump_config() { LOG_SWITCH("", "JbdBmsBle Switch", this); }
void JbdSwitch::write_state(bool state) {
  this->parent_->change_mosfet_status(this->holding_register_, this->bitmask_, state);
}

}  // namespace jbd_bms_ble
}  // namespace esphome
