#include "jbd_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace jbd_bms {

static const char *const TAG = "jbd_bms.select";

static const uint8_t JBD_CMD_READ = 0xA5;

void JbdSelect::dump_config() {
  LOG_SELECT(TAG, "JbdBms Select", this);
  ESP_LOGCONFIG(TAG, "  Options are:");
  auto options = this->traits.get_options();
  for (auto i = 0; i < this->mappings_.size(); i++) {
    ESP_LOGCONFIG(TAG, "    %i: %s", this->mappings_.at(i), options.at(i).c_str());
  }
}

void JbdSelect::control(const std::string &value) {
  auto idx = this->index_of(value);
  if (idx.has_value()) {
    uint8_t mapping = this->mappings_.at(idx.value());
    ESP_LOGV(TAG, "Reading register to %u", mapping);
    this->parent_->send_command(JBD_CMD_READ, mapping);
    return;
  }

  ESP_LOGW(TAG, "Invalid value %s", value.c_str());
}

}  // namespace jbd_bms
}  // namespace esphome
