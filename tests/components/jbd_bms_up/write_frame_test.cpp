#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

// ── Protocol reference ────────────────────────────────────────────────────────
//
// MOSFET control write command (UP protocol, FC=0x79):
//   addr  FC    start       end         len         payload                          CRC
//   01    79    29 02       29 04       00 06       11 4A 42 44  <mos_hi> <mos_lo>  XX XX
//
// Payload structure:
//   [0x11]          proprietary marker
//   [0x4A 0x42 0x44] "JBD" magic
//   [hi]  [lo]      new 16-bit MOSFET bitmask
//
// MOSFET bitmask (byte offset 32-33 in pack status frame):
//   bit 0 = DSG (discharging)
//   bit 1 = CHG (charging)
//   bit 2 = precharging
//   bit 3 = heat
//   bit 4 = fan
//
// Source: commented-out implementation in PR #165 (jbd_bms_up.cpp).

namespace esphome::jbd_bms_up::testing {

// ── build_frame_: MOSFET write request ───────────────────────────────────────

TEST(MosfetWriteFrameTest, StructureAndCRC) {
  TestableJbdBmsUp bus;
  // Enable both CHG (bit 1) and DSG (bit 0) → bitmask = 0x0003
  const std::vector<uint8_t> payload = {0x11, 0x4A, 0x42, 0x44, 0x00, 0x03};
  auto frame = bus.build_frame_(0x01, 0x79, 0x2902, 0x2904, payload);

  // clang-format off
  ASSERT_EQ(frame.size(), 16u);  // 8 header + 6 payload + 2 CRC
  EXPECT_EQ(frame[0],  0x01);   // address
  EXPECT_EQ(frame[1],  0x79);   // function code (write)
  EXPECT_EQ(frame[2],  0x29);   // start addr high
  EXPECT_EQ(frame[3],  0x02);   // start addr low
  EXPECT_EQ(frame[4],  0x29);   // end addr high
  EXPECT_EQ(frame[5],  0x04);   // end addr low
  EXPECT_EQ(frame[6],  0x00);   // data length high
  EXPECT_EQ(frame[7],  0x06);   // data length low (6 bytes)
  EXPECT_EQ(frame[8],  0x11);   // proprietary marker
  EXPECT_EQ(frame[9],  0x4A);   // 'J'
  EXPECT_EQ(frame[10], 0x42);   // 'B'
  EXPECT_EQ(frame[11], 0x44);   // 'D'
  EXPECT_EQ(frame[12], 0x00);   // MOSFET bitmask high
  EXPECT_EQ(frame[13], 0x03);   // MOSFET bitmask low (CHG + DSG)
  // clang-format on

  // CRC16 covers all 14 header+payload bytes, stored LSB-first
  uint16_t expected_crc = crc16(frame.data(), 14);
  uint16_t frame_crc = frame[14] | (uint16_t(frame[15]) << 8);
  EXPECT_EQ(frame_crc, expected_crc);
}

TEST(MosfetWriteFrameTest, DisableCharging) {
  TestableJbdBmsUp bus;
  // DSG only (bit 0), CHG off → bitmask = 0x0001
  const std::vector<uint8_t> payload = {0x11, 0x4A, 0x42, 0x44, 0x00, 0x01};
  auto frame = bus.build_frame_(0x01, 0x79, 0x2902, 0x2904, payload);

  ASSERT_EQ(frame.size(), 16u);
  EXPECT_EQ(frame[12], 0x00);
  EXPECT_EQ(frame[13], 0x01);

  uint16_t expected_crc = crc16(frame.data(), 14);
  uint16_t frame_crc = frame[14] | (uint16_t(frame[15]) << 8);
  EXPECT_EQ(frame_crc, expected_crc);
}

TEST(MosfetWriteFrameTest, BothOff) {
  TestableJbdBmsUp bus;
  // All MOSFETs off → bitmask = 0x0000
  const std::vector<uint8_t> payload = {0x11, 0x4A, 0x42, 0x44, 0x00, 0x00};
  auto frame = bus.build_frame_(0x01, 0x79, 0x2902, 0x2904, payload);

  ASSERT_EQ(frame.size(), 16u);
  EXPECT_EQ(frame[12], 0x00);
  EXPECT_EQ(frame[13], 0x00);

  uint16_t expected_crc = crc16(frame.data(), 14);
  uint16_t frame_crc = frame[14] | (uint16_t(frame[15]) << 8);
  EXPECT_EQ(frame_crc, expected_crc);
}

}  // namespace esphome::jbd_bms_up::testing
