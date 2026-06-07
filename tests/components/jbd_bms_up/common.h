#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/jbd_bms_up/jbd_bms_up.h"

namespace esphome::jbd_bms_up::testing {

class TestableJbdBmsUp : public JbdBmsUp {
 public:
  using JbdBmsUp::build_frame_;
  using JbdBmsUp::parse_byte_;
  using JbdBmsUp::rx_buffer_;

  void feed_frame(const std::vector<uint8_t> &frame) {
    for (uint8_t b : frame)
      parse_byte_(b);
    rx_buffer_.clear();
  }
};

}  // namespace esphome::jbd_bms_up::testing
