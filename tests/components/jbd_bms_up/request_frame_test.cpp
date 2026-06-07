#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_up::testing {

// ── CRC16 ─────────────────────────────────────────────────────────────────────
//
// CRC16: poly=0xA001, init=0xFFFF, LSB-first (Modbus-style).
// Expected values verified against real hardware captures in:
//   docs/pdus/JBD-UP16S020-two-devices.txt

TEST(CRC16Test, PackStatusRequestAddr1) {
  // Request addr=1: 01 78 10 00 10 a0 00 00 | 7f b2
  const uint8_t data[] = {0x01, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x00};
  EXPECT_EQ(crc16(data, 8), 0xB27F);
}

TEST(CRC16Test, PackStatusRequestAddr2) {
  // Request addr=2: 02 78 10 00 10 a0 00 00 | 3f a7
  const uint8_t data[] = {0x02, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x00};
  EXPECT_EQ(crc16(data, 8), 0xA73F);
}

TEST(CRC16Test, PackParametersRequest) {
  // Request addr=1: 01 78 1c 00 1c a0 00 00 | 7c 2e
  const uint8_t data[] = {0x01, 0x78, 0x1c, 0x00, 0x1c, 0xa0, 0x00, 0x00};
  EXPECT_EQ(crc16(data, 8), 0x2E7C);
}

TEST(CRC16Test, DatetimeGetRequest) {
  // Request addr=1: 01 78 28 00 28 0f 00 00 | 46 4b
  const uint8_t data[] = {0x01, 0x78, 0x28, 0x00, 0x28, 0x0f, 0x00, 0x00};
  EXPECT_EQ(crc16(data, 8), 0x4B46);
}

// ── build_frame_ — pack status poll ──────────────────────────────────────────
//
// Request frames verified against real hardware captures.

TEST(BuildFrameTest, PackStatusRequestAddr1) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1000, 0x10A0);
  const std::vector<uint8_t> expected = {
      0x01, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x00, 0x7f, 0xb2,
  };
  EXPECT_EQ(f, expected);
}

TEST(BuildFrameTest, PackStatusRequestAddr2) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x02, 0x78, 0x1000, 0x10A0);
  const std::vector<uint8_t> expected = {
      0x02, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x00, 0x3f, 0xa7,
  };
  EXPECT_EQ(f, expected);
}

TEST(BuildFrameTest, PackParametersRequest) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1C00, 0x1CA0);
  const std::vector<uint8_t> expected = {
      0x01, 0x78, 0x1c, 0x00, 0x1c, 0xa0, 0x00, 0x00, 0x7c, 0x2e,
  };
  EXPECT_EQ(f, expected);
}

TEST(BuildFrameTest, DatetimeGetRequest) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x2800, 0x280F);
  const std::vector<uint8_t> expected = {
      0x01, 0x78, 0x28, 0x00, 0x28, 0x0f, 0x00, 0x00, 0x46, 0x4b,
  };
  EXPECT_EQ(f, expected);
}

// ── build_frame_ — frame structure ────────────────────────────────────────────

TEST(BuildFrameTest, ReadFrameLength) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1000, 0x10A0);
  EXPECT_EQ(f.size(), 10u);
}

TEST(BuildFrameTest, ReadFrameDataLenZero) {
  TestableJbdBmsUp bus;
  auto f = bus.build_frame_(0x01, 0x78, 0x1000, 0x10A0);
  EXPECT_EQ(f[6], 0x00);
  EXPECT_EQ(f[7], 0x00);
}

TEST(BuildFrameTest, WriteFrameWithPayload) {
  // MOS control: addr=1, FC=0x79, start=0x2902, end=0x2904, payload=6 bytes
  TestableJbdBmsUp bus;
  const std::vector<uint8_t> payload = {0x11, 0x4a, 0x42, 0x44, 0x00, 0x01};
  auto f = bus.build_frame_(0x01, 0x79, 0x2902, 0x2904, payload);
  EXPECT_EQ(f.size(), 16u);
  EXPECT_EQ(f[6], 0x00);
  EXPECT_EQ(f[7], 0x06);
  EXPECT_EQ(f[8], 0x11);
  EXPECT_EQ(f[9], 0x4a);
  EXPECT_EQ(f[10], 0x42);
  EXPECT_EQ(f[11], 0x44);
}

// ── parse_byte_ — full frame roundtrip ────────────────────────────────────────

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

TEST(ParseByteTest, PackStatusFullFrameAddr1) {
  TestableJbdBmsUp bus;
  MockDevice device;
  device.set_address(0x01);
  bus.register_device(&device);

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

TEST(ParseByteTest, PackStatusFullFrameAddr2) {
  TestableJbdBmsUp bus;
  MockDevice device;
  device.set_address(0x02);
  bus.register_device(&device);

  bool any_reset = false;
  for (size_t i = 0; i < PACK_STATUS_RESPONSE_ADDR2.size(); i++) {
    bool ok = bus.parse_byte_(PACK_STATUS_RESPONSE_ADDR2[i]);
    if (!ok && i < PACK_STATUS_RESPONSE_ADDR2.size() - 1)
      any_reset = true;
  }

  EXPECT_FALSE(any_reset) << "Buffer was unexpectedly reset mid-frame";
  EXPECT_EQ(device.call_count, 1);
  EXPECT_EQ(device.received_function, 0x78);
  EXPECT_EQ(device.received_start_addr, 0x1000);
  EXPECT_EQ(device.received_data.size(), 150u);
}

TEST(ParseByteTest, MultiDeviceAddressFiltering) {
  // Two devices on the same bus — each frame must only be dispatched to the
  // device whose address matches the frame's address byte.
  // feed_frame() mirrors parse_read_(): clears rx_buffer_ after each complete frame.
  TestableJbdBmsUp bus;
  MockDevice device1, device2;
  device1.set_address(0x01);
  device2.set_address(0x02);
  bus.register_device(&device1);
  bus.register_device(&device2);

  // addr=1 frame → only device1 receives it
  bus.feed_frame(PACK_STATUS_RESPONSE);
  EXPECT_EQ(device1.call_count, 1);
  EXPECT_EQ(device2.call_count, 0);

  // addr=2 frame → only device2 receives it
  bus.feed_frame(PACK_STATUS_RESPONSE_ADDR2);
  EXPECT_EQ(device1.call_count, 1);  // unchanged
  EXPECT_EQ(device2.call_count, 1);
}

TEST(ParseByteTest, Addr1ResponseNotDispatchedToAddr2Device) {
  // A device registered at addr=2 must not receive frames addressed to addr=1.
  TestableJbdBmsUp bus;
  MockDevice device2;
  device2.set_address(0x02);
  bus.register_device(&device2);

  bus.feed_frame(PACK_STATUS_RESPONSE);

  EXPECT_EQ(device2.call_count, 0);
}

TEST(ParseByteTest, InvalidAddressRejected) {
  TestableJbdBmsUp bus;
  bool ok = bus.parse_byte_(0xFF);
  EXPECT_FALSE(ok);
}

TEST(ParseByteTest, WrongCrcRejected) {
  TestableJbdBmsUp bus;
  MockDevice device;
  device.set_address(0x01);
  bus.register_device(&device);

  std::vector<uint8_t> corrupted = PACK_STATUS_RESPONSE;
  corrupted[corrupted.size() - 1] ^= 0xFF;

  for (uint8_t b : corrupted)
    bus.parse_byte_(b);

  EXPECT_EQ(device.call_count, 0) << "Corrupted frame must not be dispatched";
}

}  // namespace esphome::jbd_bms_up::testing
