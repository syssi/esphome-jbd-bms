#pragma once
#include <cstdint>
#include <vector>
#include "esphome/components/jbd_bms_up_pack/jbd_bms_up_pack.h"

namespace esphome::jbd_bms_up_pack::testing {

class TestableJbdBmsUpPack : public JbdBmsUpPack {
 public:
  using JbdBmsUpPack::on_pack_status_;
  using JbdBmsUpPack::mosfet_status_;
  using JbdBmsUpPack::change_mosfet_status;

  void send(uint8_t function, uint16_t start_addr, uint16_t end_addr, const std::vector<uint8_t> &data) override {}
};

}  // namespace esphome::jbd_bms_up_pack::testing
