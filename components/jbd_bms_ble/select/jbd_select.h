#pragma once

#include <utility>
#include <map>

#include "../jbd_bms_ble.h"
#include "esphome/core/component.h"
#include "esphome/components/select/select.h"

namespace esphome {
namespace jbd_bms_ble {

class JbdBmsBle;
class JbdSelect : public select::Select, public Component {
 public:
  void set_parent(JbdBmsBle *parent) { this->parent_ = parent; };
  void set_select_mappings(std::vector<uint8_t> mappings) { this->mappings_ = std::move(mappings); }

  void dump_config() override;

 protected:
  void control(const std::string &value) override;

  std::vector<uint8_t> mappings_;
  JbdBmsBle *parent_;
};

}  // namespace jbd_bms_ble
}  // namespace esphome
