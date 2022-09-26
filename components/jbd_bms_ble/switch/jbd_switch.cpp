#include "jbd_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace jbd_bms_ble {

static const char *const TAG = "jbd_bms_ble.switch";

void JbdSwitch::dump_config() { LOG_SWITCH("", "JbdBmsBle Switch", this); }
void JbdSwitch::write_state(bool state) {
  // this->parent_->write_register(this->holding_register_, (uint16_t) state);
}

}  // namespace jbd_bms_ble
}  // namespace esphome
