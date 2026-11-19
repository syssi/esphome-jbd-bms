#include "jbd_button.h"

namespace esphome::jbd_bms {

void JbdButton::press_action() {
  this->parent_->send_command(this->command_, this->address_, this->payload_, this->length_);
}

}  // namespace esphome::jbd_bms
