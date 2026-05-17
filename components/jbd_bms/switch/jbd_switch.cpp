#include "jbd_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome::jbd_bms {

static const char *const TAG = "jbd_bms.switch";

void JbdSwitch::dump_config() { LOG_SWITCH("", "JbdBms Switch", this); }
void JbdSwitch::write_state(bool state) {
  if (this->parent_->change_mosfet_status(this->holding_register_, this->bitmask_, state)) {
    this->publish_state(state);
  }

  // 0xDD 0x5A 0x01 0x02 0x00 0x00 0xFF 0xFD 0x77
  // this->parent_->write_register(0x01, 0x0000);
}

void JbdBalancerSwitch::dump_config() { LOG_SWITCH("", "JbdBms Balancer Switch", this); }
void JbdBalancerSwitch::write_state(bool state) {
  if (this->parent_->change_balancer_status(state)) {
    this->publish_state(state);
  }
}

}  // namespace esphome::jbd_bms
