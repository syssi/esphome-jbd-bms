#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/jbd_bms_up/jbd_bms_up.h"

namespace esphome::jbd_bms_up::testing {

class TestableJbdBmsUp : public JbdBmsUp {
 public:
  using JbdBmsUp::build_frame_;
  using JbdBmsUp::parse_byte_;
};

}  // namespace esphome::jbd_bms_up::testing
