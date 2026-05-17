#include <gtest/gtest.h>
#include <vector>
#include "common.h"

namespace esphome::jbd_bms::testing {

// JBD write frame layout (variable length):
//   [0]        SOF         0xDD
//   [1]        CMD         0x5A (write) / 0xA5 (read)
//   [2]        address
//   [3]        data_len    n
//   [4..4+n-1] data
//   [4+n]      CRC high
//   [5+n]      CRC low
//   [6+n]      EOF         0x77
//
// CRC = 0x10000 - sum(frame[2..3+n])   (two's complement, 16-bit)
//
// Physical PulseView captures (register 0xE1 = MOS control):
//   Both ON  (0x0000): DD 5A E1 02 00 00 FF 1D 77   (initial state: charging+discharging enabled)
//   Chg OFF  (0x0001): DD 5A E1 02 00 01 FF 1C 77
//   Dsg OFF  (0x0002): DD 5A E1 02 00 02 FF 1B 77
//   Both OFF (0x0003): DD 5A E1 02 00 03 FF 1A 77

// ── Frame header / trailer ────────────────────────────────────────────────────

TEST(WriteCommandTest, HeaderAndTrailer) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x00, 0x00};
  auto f = bms.build_frame_(0x5A, 0xE1, data, 2);

  EXPECT_EQ(f[0], 0xDD);  // SOF
  EXPECT_EQ(f[1], 0x5A);  // CMD_WRITE
  EXPECT_EQ(f[3], 0x02);  // data length
  EXPECT_EQ(f[8], 0x77);  // EOF
}

// ── CRC ───────────────────────────────────────────────────────────────────────

TEST(WriteCommandTest, CrcBothEnabled) {
  TestableJbdBms bms;
  // sum(E1, 02, 00, 00) = 0xE3 → CRC = 0x10000 - 0xE3 = 0xFF1D
  const uint8_t data[2] = {0x00, 0x00};
  auto f = bms.build_frame_(0x5A, 0xE1, data, 2);
  EXPECT_EQ(f[6], 0xFF);
  EXPECT_EQ(f[7], 0x1D);
}

TEST(WriteCommandTest, CrcMatchesTwosComplement) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x00, 0x01};
  auto f = bms.build_frame_(0x5A, 0xE1, data, 2);

  uint16_t sum = 0;
  for (int i = 2; i <= 5; i++)
    sum += f[i];
  uint16_t expected = static_cast<uint16_t>(0x10000u - sum);

  EXPECT_EQ(f[6], expected >> 8);
  EXPECT_EQ(f[7], expected & 0xFF);
}

// ── Full frame comparisons against physical captures ─────────────────────────

TEST(WriteCommandTest, FrameBothEnabled) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x00, 0x00};
  auto f = bms.build_frame_(0x5A, 0xE1, data, 2);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x00, 0xFF, 0x1D, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(WriteCommandTest, FrameChargingDisabled) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x00, 0x01};
  auto f = bms.build_frame_(0x5A, 0xE1, data, 2);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x01, 0xFF, 0x1C, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(WriteCommandTest, FrameDischargingDisabled) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x00, 0x02};
  auto f = bms.build_frame_(0x5A, 0xE1, data, 2);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x02, 0xFF, 0x1B, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(WriteCommandTest, FrameBothDisabled) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x00, 0x03};
  auto f = bms.build_frame_(0x5A, 0xE1, data, 2);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0x5A, 0xE1, 0x02, 0x00, 0x03, 0xFF, 0x1A, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

// ── change_mosfet_status state transitions ───────────────────────────────────
//
// Register 0xE1 uses inverted encoding: bit set = DISABLED.
// Internal mosfet_status_ bit layout: bit0=charging, bit1=discharging.
// Wire encoding: bits XOR'd with 0x03 (both inverted).
//
// Initial state from hardware info frame: 0x03 = both enabled → wire value 0x00.

TEST(MosfetSwitchTest, ChargingOffFromBothEnabled) {
  TestableJbdBms bms;
  bms.set_mosfet_status(0x03);  // charging=1, discharging=1
  bms.change_mosfet_status(0xE1, 0, false);

  // mosfet_status_ becomes 0x02 (discharging=1, charging=0)
  // wire value: 0x02 XOR 0x03 = 0x01
  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0001);
}

TEST(MosfetSwitchTest, DischargingOffFromBothEnabled) {
  TestableJbdBms bms;
  bms.set_mosfet_status(0x03);
  bms.change_mosfet_status(0xE1, 1, false);

  // mosfet_status_ becomes 0x01 (discharging=0, charging=1)
  // wire value: 0x01 XOR 0x03 = 0x02
  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0002);
}

TEST(MosfetSwitchTest, BothOffFromBothEnabled) {
  TestableJbdBms bms;
  bms.set_mosfet_status(0x03);
  bms.change_mosfet_status(0xE1, 0, false);  // charging off → mosfet_status_=0x02
  bms.change_mosfet_status(0xE1, 1, false);  // discharging off → mosfet_status_=0x00

  // wire value: 0x00 XOR 0x03 = 0x03
  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0003);
}

TEST(MosfetSwitchTest, ChargingOnFromBothDisabled) {
  TestableJbdBms bms;
  bms.set_mosfet_status(0x00);
  bms.change_mosfet_status(0xE1, 0, true);

  // mosfet_status_ becomes 0x01
  // wire value: 0x01 XOR 0x03 = 0x02
  EXPECT_EQ(bms.last_write_address, 0xE1);
  EXPECT_EQ(bms.last_write_value, 0x0002);
}

TEST(MosfetSwitchTest, UnknownStatusReturnsError) {
  TestableJbdBms bms;
  // mosfet_status_ defaults to 255 = unknown
  bool result = bms.change_mosfet_status(0xE1, 0, false);
  EXPECT_FALSE(result);
}

// ── Button command frames ─────────────────────────────────────────────────────
//
// Register 0x0A controls BMS operations via distinct bit patterns:
//   automatic_balancing  0x0700: DD 5A 0A 02 07 00 FF ED 77
//   clear_alarm          0x0400: DD 5A 0A 02 04 00 FF F0 77
//   force_soc_reset      0x0100: DD 5A 0A 02 01 00 FF F3 77

TEST(ButtonCommandTest, AutomaticBalancingFrame) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x07, 0x00};
  auto f = bms.build_frame_(0x5A, 0x0A, data, 2);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0x5A, 0x0A, 0x02, 0x07, 0x00, 0xFF, 0xED, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(ButtonCommandTest, ClearAlarmFrame) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x04, 0x00};
  auto f = bms.build_frame_(0x5A, 0x0A, data, 2);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0x5A, 0x0A, 0x02, 0x04, 0x00, 0xFF, 0xF0, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(ButtonCommandTest, ForceSocResetFrame) {
  TestableJbdBms bms;
  const uint8_t data[2] = {0x01, 0x00};
  auto f = bms.build_frame_(0x5A, 0x0A, data, 2);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0x5A, 0x0A, 0x02, 0x01, 0x00, 0xFF, 0xF3, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

// ── Read command frames ───────────────────────────────────────────────────────
//
// Read frames carry no payload (data_len=0); CRC covers only [address, 0x00].
//   hardware_info              reg 0x03: DD A5 03 00 FF FD 77  (confirmed: btsnoop_hci_jbd-sp04s034-l4s-200a-b-u.log)
//   cell_info                  reg 0x04: DD A5 04 00 FF FC 77  (confirmed: btsnoop_hci_jbd-sp04s034-l4s-200a-b-u.log)
//   retrieve_hardware_version  reg 0x05: DD A5 05 00 FF FB 77  (confirmed: debug.txt)
//   retrieve_error_counts      reg 0xAA: DD A5 AA 00 FF 56 77  (confirmed: btsnoop_hci_jbd-sp04s034-l4s-200a-b-u.log)

TEST(ReadCommandTest, HardwareInfoFrame) {
  TestableJbdBms bms;
  auto f = bms.build_frame_(0xA5, 0x03, nullptr, 0);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(ReadCommandTest, CellInfoFrame) {
  TestableJbdBms bms;
  auto f = bms.build_frame_(0xA5, 0x04, nullptr, 0);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(ReadCommandTest, RetrieveHardwareVersionFrame) {
  TestableJbdBms bms;
  auto f = bms.build_frame_(0xA5, 0x05, nullptr, 0);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0xA5, 0x05, 0x00, 0xFF, 0xFB, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(ReadCommandTest, RetrieveErrorCountsFrame) {
  TestableJbdBms bms;
  auto f = bms.build_frame_(0xA5, 0xAA, nullptr, 0);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0xDD, 0xA5, 0xAA, 0x00, 0xFF, 0x56, 0x77,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

}  // namespace esphome::jbd_bms::testing
