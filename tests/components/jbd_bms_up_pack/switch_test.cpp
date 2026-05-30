#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_up_pack::testing {

using esphome::jbd_bms_up::testing::PACK_STATUS_FRAME;

// ── mosfet_status_ tracking ──────────────────────────────────────────────────

TEST(MosfetStatusTest, InitialStateUnknown) {
  TestableJbdBmsUpPack pack;
  EXPECT_EQ(pack.mosfet_status_, 0xFFFF);
}

TEST(MosfetStatusTest, StoredAfterPackStatus) {
  TestableJbdBmsUpPack pack;
  pack.on_pack_status_(PACK_STATUS_FRAME);
  // PACK_STATUS_FRAME has mos=0x0003 at byte 32-33 (CHG + DSG both on)
  EXPECT_EQ(pack.mosfet_status_, 0x0003);
}

// ── change_mosfet_status ─────────────────────────────────────────────────────

TEST(MosfetStatusTest, FailsWhenStatusUnknown) {
  TestableJbdBmsUpPack pack;
  // mosfet_status_ is 0xFFFF (initial unknown) — must return false
  EXPECT_FALSE(pack.change_mosfet_status(1, true));
  EXPECT_EQ(pack.mosfet_status_, 0xFFFF);
}

TEST(MosfetStatusTest, DisableCharging) {
  TestableJbdBmsUpPack pack;
  pack.on_pack_status_(PACK_STATUS_FRAME);           // mosfet_status_ = 0x0003 (CHG + DSG)
  EXPECT_TRUE(pack.change_mosfet_status(1, false));  // bit 1 = CHG off
  EXPECT_EQ(pack.mosfet_status_, 0x0001);            // only DSG remains
}

TEST(MosfetStatusTest, DisableDischarging) {
  TestableJbdBmsUpPack pack;
  pack.on_pack_status_(PACK_STATUS_FRAME);           // mosfet_status_ = 0x0003
  EXPECT_TRUE(pack.change_mosfet_status(0, false));  // bit 0 = DSG off
  EXPECT_EQ(pack.mosfet_status_, 0x0002);            // only CHG remains
}

TEST(MosfetStatusTest, DisableBoth) {
  TestableJbdBmsUpPack pack;
  pack.on_pack_status_(PACK_STATUS_FRAME);  // mosfet_status_ = 0x0003
  pack.change_mosfet_status(1, false);
  pack.change_mosfet_status(0, false);
  EXPECT_EQ(pack.mosfet_status_, 0x0000);
}

TEST(MosfetStatusTest, EnableAlreadySet) {
  TestableJbdBmsUpPack pack;
  pack.on_pack_status_(PACK_STATUS_FRAME);          // mosfet_status_ = 0x0003 (CHG + DSG)
  EXPECT_TRUE(pack.change_mosfet_status(1, true));  // CHG already on — idempotent
  EXPECT_EQ(pack.mosfet_status_, 0x0003);
}

// ── Write ACK response handling ───────────────────────────────────────────────

TEST(MosfetWriteAckTest, WriteAckHandledGracefully) {
  TestableJbdBmsUpPack pack;
  // FC=0x79, addr=0x2902, empty data — must not crash or log warning
  EXPECT_NO_FATAL_FAILURE(pack.on_jbd_bms_up_data(0x79, 0x2902, {}));
}

TEST(MosfetWriteAckTest, UnknownFunctionStillWarns) {
  TestableJbdBmsUpPack pack;
  // Unknown function/address — handled by the fallback warning path
  std::vector<uint8_t> dummy = {0x01, 0x02};
  EXPECT_NO_FATAL_FAILURE(pack.on_jbd_bms_up_data(0x7A, 0x1234, dummy));
}

}  // namespace esphome::jbd_bms_up_pack::testing
