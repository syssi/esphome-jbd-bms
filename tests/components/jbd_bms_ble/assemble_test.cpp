#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_ble::testing {

// Wraps a payload into a raw JBD response frame:
//   0xDD <function> <status 0x00> <len> <payload...> <crc_hi> <crc_lo> 0x77
static std::vector<uint8_t> make_raw_frame(uint8_t function, const std::vector<uint8_t> &payload) {
  std::vector<uint8_t> frame = {0xDD, function, 0x00, (uint8_t) payload.size()};
  frame.insert(frame.end(), payload.begin(), payload.end());
  uint16_t sum = 0;
  for (size_t i = 2; i < frame.size(); i++)
    sum += frame[i];
  uint16_t crc = (uint16_t) (0x10000 - sum);
  frame.push_back(crc >> 8);
  frame.push_back(crc & 0xFF);
  frame.push_back(0x77);
  return frame;
}

// Feeds a raw frame to assemble() in BLE-notification-sized chunks (20 bytes with the default MTU of 23).
static void feed_chunked(TestableJbdBmsBle &bms, const std::vector<uint8_t> &frame, size_t chunk_size = 20) {
  for (size_t offset = 0; offset < frame.size(); offset += chunk_size) {
    size_t len = std::min(chunk_size, frame.size() - offset);
    bms.assemble(frame.data() + offset, len);
  }
}

TEST(JbdBmsBleAssembleTest, SingleNotificationFrame) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  auto frame = make_raw_frame(0x03, BASICINFO_FRAME);
  bms.assemble(frame.data(), frame.size());

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

TEST(JbdBmsBleAssembleTest, ChunkedFrame) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  feed_chunked(bms, make_raw_frame(0x03, BASICINFO_FRAME));

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

// A payload byte equal to JBD_PKT_END (0x77) that happens to be the last byte of a notification
// must not be mistaken for the end of the frame. Reproduces a 45-byte basic info frame
// (38-byte payload, split 20/20/5) where raw byte 39 is 0x77, which previously logged
// "Invalid frame length: expected 45, got 40" on every poll.
TEST(JbdBmsBleAssembleTest, PayloadEndByteAtChunkBoundary) {
  TestableJbdBmsBle bms;
  sensor::Sensor total, soc;
  bms.set_total_voltage_sensor(&total);
  bms.set_state_of_charge_sensor(&soc);

  // Basic info payload followed by extended fields reported by newer firmware
  std::vector<uint8_t> payload = BASICINFO_FRAME;
  payload.resize(38, 0x00);
  payload[35] = 0x77;

  auto frame = make_raw_frame(0x03, payload);
  ASSERT_EQ(frame.size(), 45u);
  ASSERT_EQ(frame[39], 0x77);

  feed_chunked(bms, frame);

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
  EXPECT_FLOAT_EQ(soc.state, 100.0f);
}

TEST(JbdBmsBleAssembleTest, CorruptedChecksumIsRejected) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  auto frame = make_raw_frame(0x03, BASICINFO_FRAME);
  frame[frame.size() - 2] ^= 0xFF;
  feed_chunked(bms, frame);

  EXPECT_TRUE(std::isnan(total.state));
}

}  // namespace esphome::jbd_bms_ble::testing
