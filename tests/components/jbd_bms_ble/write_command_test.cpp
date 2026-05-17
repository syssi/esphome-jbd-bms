#include <gtest/gtest.h>
#include <array>
#include "common.h"

namespace esphome::jbd_bms_ble::testing {

// JBD write frame layout (9 bytes):
//   [0]    SOF         0xDD
//   [1]    CMD_WRITE   0x5A
//   [2]    address
//   [3]    data_len    0x02
//   [4]    value high
//   [5]    value low
//   [6]    CRC high
//   [7]    CRC low
//   [8]    EOF         0x77
//
// CRC = 0x10000 - sum(frame[2..5])   (two's complement, 16-bit)
//
// Physical PulseView captures (register 0xE1 = MOS control):
//   Both ON  (0x0000): DD 5A E1 02 00 00 FF 1D 77
//   Chg OFF  (0x0001): DD 5A E1 02 00 01 FF 1C 77
//   Dsg OFF  (0x0002): DD 5A E1 02 00 02 FF 1B 77
//   Both OFF (0x0003): DD 5A E1 02 00 03 FF 1A 77

// ── Frame header / trailer ────────────────────────────────────────────────────

TEST(WriteCommandTest, HeaderAndTrailer) {
  TestableJbdBmsBle bms;
  auto f = bms.build_frame_(0x5A, 0xE1, 0x0000);

  EXPECT_EQ(f[0], 0xDD);  // SOF
  EXPECT_EQ(f[1], 0x5A);  // CMD_WRITE
  EXPECT_EQ(f[3], 0x02);  // data length
  EXPECT_EQ(f[8], 0x77);  // EOF
}

// ── CRC ───────────────────────────────────────────────────────────────────────

TEST(WriteCommandTest, CrcBothEnabled) {
  TestableJbdBmsBle bms;
  // sum(E1, 02, 00, 00) = 0xE3 → CRC = 0x10000 - 0xE3 = 0xFF1D
  auto f = bms.build_frame_(0x5A, 0xE1, 0x0000);
  EXPECT_EQ(f[6], 0xFF);
  EXPECT_EQ(f[7], 0x1D);
}

TEST(WriteCommandTest, CrcMatchesTwosComplement) {
  TestableJbdBmsBle bms;
  auto f = bms.build_frame_(0x5A, 0xE1, 0x0001);

  uint16_t sum = 0;
  for (int i = 2; i <= 5; i++)
    sum += f[i];
  uint16_t expected = static_cast<uint16_t>(0x10000u - sum);

  EXPECT_EQ(f[6], expected >> 8);
  EXPECT_EQ(f[7], expected & 0xFF);
}

// ── Full frame comparisons against physical captures ─────────────────────────

TEST(WriteCommandTest, FrameBothEnabled) {
  TestableJbdBmsBle bms;
  auto f = bms.build_frame_(0x5A, 0xE1, 0x0000);

  // clang-format off
  const std::array<uint8_t, 9> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x00, 0xFF, 0x1D, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(WriteCommandTest, FrameChargingDisabled) {
  TestableJbdBmsBle bms;
  auto f = bms.build_frame_(0x5A, 0xE1, 0x0001);

  // clang-format off
  const std::array<uint8_t, 9> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x01, 0xFF, 0x1C, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(WriteCommandTest, FrameDischargingDisabled) {
  TestableJbdBmsBle bms;
  auto f = bms.build_frame_(0x5A, 0xE1, 0x0002);

  // clang-format off
  const std::array<uint8_t, 9> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x02, 0xFF, 0x1B, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(WriteCommandTest, FrameBothDisabled) {
  TestableJbdBmsBle bms;
  auto f = bms.build_frame_(0x5A, 0xE1, 0x0003);

  // clang-format off
  const std::array<uint8_t, 9> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x03, 0xFF, 0x1A, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

// ── change_mosfet_status state transitions ───────────────────────────────────

TEST(MosfetSwitchTest, ChargingOffFromBothEnabled) {
  TestableJbdBmsBle bms;
  bms.set_mosfet_status(0x03);
  bms.change_mosfet_status(0xE1, 0, false);

  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0001);
}

TEST(MosfetSwitchTest, DischargingOffFromBothEnabled) {
  TestableJbdBmsBle bms;
  bms.set_mosfet_status(0x03);
  bms.change_mosfet_status(0xE1, 1, false);

  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0002);
}

TEST(MosfetSwitchTest, BothOffFromBothEnabled) {
  TestableJbdBmsBle bms;
  bms.set_mosfet_status(0x03);
  bms.change_mosfet_status(0xE1, 0, false);  // charging off → mosfet_status_=0x02
  bms.change_mosfet_status(0xE1, 1, false);  // discharging off → mosfet_status_=0x00

  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0003);
}

TEST(MosfetSwitchTest, ChargingOnFromBothDisabled) {
  TestableJbdBmsBle bms;
  bms.set_mosfet_status(0x00);
  bms.change_mosfet_status(0xE1, 0, true);

  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0002);
}

TEST(MosfetSwitchTest, UnknownStatusReturnsError) {
  TestableJbdBmsBle bms;
  bool result = bms.change_mosfet_status(0xE1, 0, false);
  EXPECT_FALSE(result);
}

}  // namespace esphome::jbd_bms_ble::testing
