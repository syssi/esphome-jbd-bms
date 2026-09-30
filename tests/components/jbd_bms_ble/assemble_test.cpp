#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_ble::testing {

// Wraps a payload into a raw response frame: 0xDD <function> <status 0x00> <len> <payload...> <crc> 0x77
//
// Only meant for the synthetic frame below. Everything else uses the captured *_RAW_FRAME
// constants, so a bug in this helper can't hide a bug in assemble().
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

// ── Single fragment (MTU large enough for the whole frame) ────────────────────

TEST(JbdBmsBleAssembleTest, SingleFragmentDecodesBasicInfo) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.assemble(BASICINFO_RAW_FRAME.data(), BASICINFO_RAW_FRAME.size());

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

TEST(JbdBmsBleAssembleTest, CorruptedCrcIsRejected) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  std::vector<uint8_t> corrupted = BASICINFO_RAW_FRAME;
  corrupted[corrupted.size() - 2] ^= 0xFF;  // flip the low CRC byte

  bms.assemble(corrupted.data(), corrupted.size());

  EXPECT_TRUE(std::isnan(total.state));  // never decoded, sensor keeps its unset state
}

// ── Small-MTU fragmentation ─────────────────────────────────────────────────
//
// Regression coverage for https://github.com/syssi/esphome-jbd-bms/pull/246: with the default
// MTU of 23 a response arrives in 20-byte notifications. The old assemble() treated a frame as
// complete as soon as the accumulated buffer happened to end on 0x77 (JBD_PKT_END) -- but 0x77
// also occurs as an ordinary payload byte, so a fragment boundary landing there produced
// "Invalid frame length: expected 45, got 40" and dropped the whole frame on every poll.

TEST(JbdBmsBleAssembleTest, StrayEndByteAtFragmentBoundaryDoesNotTerminateFrameEarly) {
  // SYNTHETIC frame, not a capture: the 4S basic info payload zero-padded to the 38 bytes
  // reported by the affected 16S BMS (PDEE02905J-222), with 0x77 placed at the byte that ends
  // the second notification. Kept local to this test on purpose, don't reuse it for decoding tests.
  std::vector<uint8_t> payload = BASICINFO_FRAME;
  payload.resize(38, 0x00);
  payload[35] = 0x77;
  std::vector<uint8_t> frame = make_raw_frame(0x03, payload);

  ASSERT_EQ(frame.size(), 45u);  // arrives as 20 + 20 + 5
  ASSERT_EQ(frame[39], 0x77);    // the byte that used to trigger the bug

  TestableJbdBmsBle bms;
  sensor::Sensor total, soc;
  bms.set_total_voltage_sensor(&total);
  bms.set_state_of_charge_sensor(&soc);

  const size_t mtu_payload = 20;
  for (size_t offset = 0; offset < frame.size(); offset += mtu_payload) {
    size_t chunk = std::min(mtu_payload, frame.size() - offset);
    bms.assemble(&frame[offset], chunk);
  }

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
  EXPECT_FLOAT_EQ(soc.state, 100.0f);
}

TEST(JbdBmsBleAssembleTest, IncompleteFrameDoesNotDecodeYet) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.assemble(BASICINFO_RAW_FRAME.data(), 20);  // first notification only
  EXPECT_TRUE(std::isnan(total.state));          // still waiting for the remaining 16 bytes

  bms.assemble(BASICINFO_RAW_FRAME.data() + 20, BASICINFO_RAW_FRAME.size() - 20);
  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

TEST(JbdBmsBleAssembleTest, RealisticSmallMtuFragmentsReassembleBasicInfo) {
  TestableJbdBmsBle bms;
  sensor::Sensor total, soc;
  bms.set_total_voltage_sensor(&total);
  bms.set_state_of_charge_sensor(&soc);

  const size_t mtu_payload = 20;
  const auto &frame = BASICINFO_RAW_FRAME;
  for (size_t offset = 0; offset < frame.size(); offset += mtu_payload) {
    size_t chunk = std::min(mtu_payload, frame.size() - offset);
    bms.assemble(&frame[offset], chunk);
  }

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
  EXPECT_FLOAT_EQ(soc.state, 100.0f);
}

TEST(JbdBmsBleAssembleTest, OddSizedFragmentsReassembleBasicInfo) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  // Fragment size that doesn't evenly divide the frame.
  const size_t mtu_payload = 7;
  const auto &frame = BASICINFO_RAW_FRAME;
  for (size_t offset = 0; offset < frame.size(); offset += mtu_payload) {
    size_t chunk = std::min(mtu_payload, frame.size() - offset);
    bms.assemble(&frame[offset], chunk);
  }

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

TEST(JbdBmsBleAssembleTest, CorruptedCrcInFragmentedFrameIsRejected) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  std::vector<uint8_t> corrupted = BASICINFO_RAW_FRAME;
  corrupted[corrupted.size() - 2] ^= 0xFF;  // flip the low CRC byte

  const size_t mtu_payload = 20;
  for (size_t offset = 0; offset < corrupted.size(); offset += mtu_payload) {
    size_t chunk = std::min(mtu_payload, corrupted.size() - offset);
    bms.assemble(&corrupted[offset], chunk);
  }

  EXPECT_TRUE(std::isnan(total.state));
}

// ── Missing end of frame marker ───────────────────────────────────────────────

TEST(JbdBmsBleAssembleTest, MissingEndOfFrameMarkerIsDropped) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  std::vector<uint8_t> broken = BASICINFO_RAW_FRAME;
  broken.back() = 0x00;

  bms.assemble(broken.data(), broken.size());
  EXPECT_TRUE(std::isnan(total.state));

  bms.assemble(BASICINFO_RAW_FRAME.data(), BASICINFO_RAW_FRAME.size());
  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

// ── Resync after a lost notification ──────────────────────────────────────────

TEST(JbdBmsBleAssembleTest, FreshFrameStartResyncsAfterAbandonedFrame) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  // The second notification of a basic info frame is lost...
  bms.assemble(BASICINFO_RAW_FRAME.data(), 20);
  // ...then the response to the next poll arrives. The stale half must be discarded, not prepended.
  bms.assemble(BASICINFO_RAW_FRAME.data(), BASICINFO_RAW_FRAME.size());

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

// ── Multiple frames in sequence ───────────────────────────────────────────────

TEST(JbdBmsBleAssembleTest, BasicInfoThenCellInfoBothDecode) {
  TestableJbdBmsBle bms;
  sensor::Sensor total, cell1;
  bms.set_total_voltage_sensor(&total);
  bms.set_cell_voltage_sensor(0, &cell1);

  bms.assemble(BASICINFO_RAW_FRAME.data(), BASICINFO_RAW_FRAME.size());
  bms.assemble(CELLINFO_RAW_FRAME.data(), CELLINFO_RAW_FRAME.size());

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
  EXPECT_NEAR(cell1.state, 3.909f, 0.001f);
}

}  // namespace esphome::jbd_bms_ble::testing
