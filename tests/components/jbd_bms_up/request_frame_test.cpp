#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_up::testing {

// ── CRC16 ─────────────────────────────────────────────────────────────────────
//
// CRC16: poly=0xA001, init=0xFFFF, LSB-first (Modbus-style).
// Expected values verified against hardware captures in:
//   https://gist.github.com/PhracturedBlue/7ef619594eaa4c27f4ff068b461865b8

TEST(CRC16Test, PackStatusRequest) {
  // Request: 01 78 10 00 10 a0 00 00 | 7f b2
  // CRC stored LSB-first: 0x7F, 0xB2 → value = 0x7F | (0xB2 << 8) = 0xB27F
  const uint8_t data[] = {0x01, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x00};
  EXPECT_EQ(crc16(data, 8), 0xB27F);
}

TEST(CRC16Test, PackParametersRequest) {
  // Request: 01 78 1c 00 1c a0 00 00 | 7c 2e
  // CRC: 0x7C | (0x2E << 8) = 0x2E7C
  const uint8_t data[] = {0x01, 0x78, 0x1c, 0x00, 0x1c, 0xa0, 0x00, 0x00};
  EXPECT_EQ(crc16(data, 8), 0x2E7C);
}

TEST(CRC16Test, DatetimeGetRequest) {
  // Request: 01 78 28 00 28 0f 00 00 | 46 4b
  // CRC: 0x46 | (0x4B << 8) = 0x4B46
  const uint8_t data[] = {0x01, 0x78, 0x28, 0x00, 0x28, 0x0f, 0x00, 0x00};
  EXPECT_EQ(crc16(data, 8), 0x4B46);
}

// ── build_frame_ — pack status poll ──────────────────────────────────────────
//
// Pack status request (addr=1, FC=0x78, start=0x1000, end=0x10A0, no payload):
//   01 78 10 00 10 a0 00 00 7f b2
// Source: gist hardware capture.

TEST(BuildFrameTest, PackStatusRequest) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1000, 0x10A0);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0x01, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x00, 0x7f, 0xb2,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(BuildFrameTest, PackParametersRequest) {
  // Request: 01 78 1c 00 1c a0 00 00 7c 2e
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1C00, 0x1CA0);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0x01, 0x78, 0x1c, 0x00, 0x1c, 0xa0, 0x00, 0x00, 0x7c, 0x2e,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(BuildFrameTest, DatetimeGetRequest) {
  // Request: 01 78 28 00 28 0f 00 00 46 4b
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x2800, 0x280F);

  // clang-format off
  const std::vector<uint8_t> expected = {
      0x01, 0x78, 0x28, 0x00, 0x28, 0x0f, 0x00, 0x00, 0x46, 0x4b,
  };
  // clang-format on
  EXPECT_EQ(f, expected);
}

TEST(BuildFrameTest, Address2Request) {
  // Same poll but for address 2 — CRC must change
  TestableJbdBmsUp bus;
  auto f1 = bus.build_frame_(0x01, 0x78, 0x1000, 0x10A0);
  auto f2 = bus.build_frame_(0x02, 0x78, 0x1000, 0x10A0);
  // Different address → different CRC
  EXPECT_NE(f1[8], f2[8]);
  EXPECT_EQ(f2[0], 0x02);
}

// ── build_frame_ — frame structure ────────────────────────────────────────────

TEST(BuildFrameTest, ReadFrameLength) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1000, 0x10A0);
  // 8 header + 0 payload + 2 CRC = 10 bytes
  EXPECT_EQ(f.size(), 10u);
}

TEST(BuildFrameTest, ReadFrameDataLenZero) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1000, 0x10A0);
  EXPECT_EQ(f[6], 0x00);
  EXPECT_EQ(f[7], 0x00);
}

TEST(BuildFrameTest, WriteFrameWithPayload) {
  // MOS control: 01 79 29 02 29 04 00 06 11 4a 42 44 00 01 [CRC]
  TestableJbdBmsUp bus;
  const std::vector<uint8_t> payload = {0x11, 0x4a, 0x42, 0x44, 0x00, 0x01};
  auto f = bus.build_frame_(0x01, 0x79, 0x2902, 0x2904, payload);
  // 8 header + 6 payload + 2 CRC = 16 bytes
  EXPECT_EQ(f.size(), 16u);
  // data_len field = 0x0006
  EXPECT_EQ(f[6], 0x00);
  EXPECT_EQ(f[7], 0x06);
  // payload starts at byte 8
  EXPECT_EQ(f[8], 0x11);
  EXPECT_EQ(f[9], 0x4a);
  EXPECT_EQ(f[10], 0x42);
  EXPECT_EQ(f[11], 0x44);
}

// ── parse_byte_ — full frame roundtrip ────────────────────────────────────────
//
// Feed the captured pack status response byte by byte into parse_byte_,
// verify that on_jbd_bms_up_data is called exactly once with the right args.

class MockDevice : public JbdBmsUpDevice {
 public:
  void on_jbd_bms_up_data(uint8_t function, uint16_t start_addr, const std::vector<uint8_t> &data) override {
    received_function = function;
    received_start_addr = start_addr;
    received_data = data;
    call_count++;
  }
  uint8_t received_function{0};
  uint16_t received_start_addr{0};
  std::vector<uint8_t> received_data;
  int call_count{0};
};

TEST(ParseByteTest, PackStatusFullFrame) {
  using esphome::jbd_bms_up::testing::PACK_STATUS_RESPONSE;

  TestableJbdBmsUp bus;
  MockDevice device;
  device.set_address(0x01);
  bus.register_device(&device);

  // Feed all bytes — last byte triggers CRC check and dispatch
  bool any_reset = false;
  for (size_t i = 0; i < PACK_STATUS_RESPONSE.size(); i++) {
    bool ok = bus.parse_byte_(PACK_STATUS_RESPONSE[i]);
    if (!ok && i < PACK_STATUS_RESPONSE.size() - 1)
      any_reset = true;
  }

  EXPECT_FALSE(any_reset) << "Buffer was unexpectedly reset mid-frame";
  EXPECT_EQ(device.call_count, 1);
  EXPECT_EQ(device.received_function, 0x78);
  EXPECT_EQ(device.received_start_addr, 0x1000);
  EXPECT_EQ(device.received_data.size(), 158u);
}

TEST(ParseByteTest, InvalidAddressRejected) {
  TestableJbdBmsUp bus;
  // Address 0xFF > 247: should be rejected at byte 0
  bool ok = bus.parse_byte_(0xFF);
  EXPECT_FALSE(ok);
}

TEST(ParseByteTest, WrongCrcRejected) {
  using esphome::jbd_bms_up::testing::PACK_STATUS_RESPONSE;

  TestableJbdBmsUp bus;
  MockDevice device;
  device.set_address(0x01);
  bus.register_device(&device);

  // Corrupt the CRC
  std::vector<uint8_t> corrupted = PACK_STATUS_RESPONSE;
  corrupted[corrupted.size() - 1] ^= 0xFF;

  for (uint8_t b : corrupted)
    bus.parse_byte_(b);

  EXPECT_EQ(device.call_count, 0) << "Corrupted frame should not be dispatched";
}

// ── @TODO: missing frame tests ────────────────────────────────────────────────
//
// The following frames are documented in the gist but no complete byte captures
// are available yet. Add tests once hardware captures with verified CRC exist.
//
// @TODO Pack Parameters response (0x1C00, FC=0x78):
//   Request:  01 78 1c 00 1c a0 00 00 7c 2e
//   Response: 136 bytes — balance voltage, thermal thresholds, BMS code
//   Needs: frames for on_pack_parameters_() decoder (not yet implemented)
//
// @TODO Alternate Pack Status (FC=0x45, addr=0x0000):
//   Request:  01 45 00 00 00 54 00 00 d4 d3
//   Response: 124 bytes — cell voltages, temperatures, capacities via 0x45
//   Needs: on_jbd_bms_up_data dispatch for FC=0x45
//
// @TODO DateTime Read response (0x2800, FC=0x78):
//   Request:  01 78 28 00 28 0f 00 00 46 4b
//   Response: 01 78 28 00 28 0f 00 0c [12 bytes: year,month,day,h,m,s] [CRC]
//   Needs: datetime sensor entities (not yet implemented)
//
// @TODO DateTime Write (0x2800, FC=0x79):
//   Request:  01 79 28 00 28 0c 00 10 11 4a 42 44 [7-byte timestamp] [CRC]
//   Needs: datetime write button/service (not yet implemented)
//
// @TODO MOS Control Write (0x2902, FC=0x79):
//   Request:  01 79 29 02 29 04 00 06 11 4a 42 44 [bitmask 2B] [CRC]
//   Example:  01 79 29 02 29 04 00 06 11 4a 42 44 00 01 15 85
//   Needs: MOS write implementation + switch entities

}  // namespace esphome::jbd_bms_up::testing
