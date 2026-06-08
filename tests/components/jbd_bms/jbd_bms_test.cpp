#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms::testing {

// ── Total voltage ─────────────────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, TotalVoltage) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_NEAR(b.total_voltage.state, 15.60f, 0.01f);
}

// ── Current and power ─────────────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, CurrentAndPower) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_NEAR(b.current.state, 0.0f, 0.001f);
  EXPECT_NEAR(b.power.state, 0.0f, 0.01f);
  EXPECT_NEAR(b.charging_power.state, 0.0f, 0.01f);
  EXPECT_NEAR(b.discharging_power.state, 0.0f, 0.01f);
}

// ── Capacity ──────────────────────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, Capacity) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_NEAR(b.capacity_remaining.state, 4.98f, 0.01f);
  EXPECT_NEAR(b.nominal_capacity.state, 5.00f, 0.01f);
  EXPECT_FLOAT_EQ(b.charging_cycles.state, 0.0f);
}

// ── State of charge ───────────────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, StateOfCharge) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_FLOAT_EQ(b.state_of_charge.state, 100.0f);
}

// ── Software version ──────────────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, SoftwareVersion) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_NEAR(b.software_version.state, 8.0f, 0.01f);
}

// ── Battery strings and temperature sensors ───────────────────────────────────

TEST(JbdBmsHwInfoTest, BatteryStringsAndTempSensors) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_FLOAT_EQ(b.battery_strings.state, 4.0f);
  EXPECT_FLOAT_EQ(b.temperature_sensors_count.state, 3.0f);
}

// ── Temperatures ──────────────────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, Temperatures) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_NEAR(b.temps[0].state, 22.4f, 0.05f);
  EXPECT_NEAR(b.temps[1].state, 22.3f, 0.05f);
  EXPECT_NEAR(b.temps[2].state, 21.7f, 0.05f);
}

// ── Balancer and error bitmasks ───────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, Bitmasks) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_FLOAT_EQ(b.balancer_status_bitmask.state, 0.0f);
  EXPECT_FLOAT_EQ(b.errors_bitmask.state, 0.0f);
  EXPECT_FLOAT_EQ(b.operation_status_bitmask.state, 3.0f);
}

// ── Binary sensors and switches ───────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, BinarySensorsAndSwitches) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_TRUE(b.charging_bs.state);
  EXPECT_TRUE(b.discharging_bs.state);
  EXPECT_FALSE(b.balancing_bs.state);
  EXPECT_TRUE(b.charging_sw.state);
  EXPECT_TRUE(b.discharging_sw.state);
}

// ── Operation status text ─────────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, OperationStatusText) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_NE(b.operation_status_ts.state.find("Charging"), std::string::npos);
  EXPECT_NE(b.operation_status_ts.state.find("Discharging"), std::string::npos);
}

// ── Errors text (no errors) ───────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, ErrorsTextEmpty) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_EQ(b.errors_ts.state, "");
}

// ── Cell voltages ─────────────────────────────────────────────────────────────

TEST(JbdBmsCellInfoTest, CellVoltages) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x04, CELLINFO_FRAME);
  EXPECT_NEAR(b.cells[0].state, 3.909f, 0.001f);
  EXPECT_NEAR(b.cells[1].state, 3.901f, 0.001f);
  EXPECT_NEAR(b.cells[2].state, 3.895f, 0.001f);
  EXPECT_NEAR(b.cells[3].state, 3.901f, 0.001f);
}

// ── Cell voltage statistics ───────────────────────────────────────────────────

TEST(JbdBmsCellInfoTest, CellVoltageStats) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x04, CELLINFO_FRAME);
  EXPECT_NEAR(b.min_cell_voltage.state, 3.895f, 0.001f);
  EXPECT_NEAR(b.max_cell_voltage.state, 3.909f, 0.001f);
  EXPECT_FLOAT_EQ(b.min_voltage_cell.state, 3.0f);
  EXPECT_FLOAT_EQ(b.max_voltage_cell.state, 1.0f);
  EXPECT_NEAR(b.delta_cell_voltage.state, 0.014f, 0.001f);
  EXPECT_NEAR(b.average_cell_voltage.state, 3.9015f, 0.001f);
}

// ── Device model ──────────────────────────────────────────────────────────────

TEST(JbdBmsHwVerTest, DeviceModel) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x05, HWVER_FRAME);
  EXPECT_EQ(b.device_model_ts.state, "JBD-SP04S034-L4S-200A-B-U");
}

// ── Error counts ──────────────────────────────────────────────────────────────

TEST(JbdBmsErrorCountsTest, ErrorCounts) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0xAA, ERRCOUNT_FRAME);
  EXPECT_FLOAT_EQ(b.short_circuit_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.charge_overcurrent_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.discharge_overcurrent_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.cell_overvoltage_error_count.state, 122.0f);
  EXPECT_FLOAT_EQ(b.cell_undervoltage_error_count.state, 2.0f);
  EXPECT_FLOAT_EQ(b.charge_overtemperature_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.charge_undertemperature_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.discharge_overtemperature_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.discharge_undertemperature_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.battery_overvoltage_error_count.state, 0.0f);
  EXPECT_FLOAT_EQ(b.battery_undervoltage_error_count.state, 0.0f);
}

// ── Protection binary sensors ─────────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, ProtectionBinarySensorsNoneActive) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_FALSE(b.cell_overvoltage_protection.state);
  EXPECT_FALSE(b.cell_undervoltage_protection.state);
  EXPECT_FALSE(b.pack_overvoltage_protection.state);
  EXPECT_FALSE(b.pack_undervoltage_protection.state);
  EXPECT_FALSE(b.charge_overtemperature_protection.state);
  EXPECT_FALSE(b.charge_undertemperature_protection.state);
  EXPECT_FALSE(b.discharge_overtemperature_protection.state);
  EXPECT_FALSE(b.discharge_undertemperature_protection.state);
  EXPECT_FALSE(b.charge_overcurrent_protection.state);
  EXPECT_FALSE(b.discharge_overcurrent_protection.state);
  EXPECT_FALSE(b.short_circuit_protection.state);
  EXPECT_FALSE(b.ic_frontend_error.state);
  EXPECT_FALSE(b.mosfet_software_lock.state);
}

TEST(JbdBmsHwInfoTest, ProtectionBinarySensorsCellOvervoltage) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME_CELL_OVERVOLTAGE);
  EXPECT_TRUE(b.cell_overvoltage_protection.state);
  EXPECT_FALSE(b.cell_undervoltage_protection.state);
  EXPECT_FALSE(b.pack_overvoltage_protection.state);
  EXPECT_FALSE(b.pack_undervoltage_protection.state);
  EXPECT_FALSE(b.charge_overtemperature_protection.state);
  EXPECT_FALSE(b.charge_undertemperature_protection.state);
  EXPECT_FALSE(b.discharge_overtemperature_protection.state);
  EXPECT_FALSE(b.discharge_undertemperature_protection.state);
  EXPECT_FALSE(b.charge_overcurrent_protection.state);
  EXPECT_FALSE(b.discharge_overcurrent_protection.state);
  EXPECT_FALSE(b.short_circuit_protection.state);
  EXPECT_FALSE(b.ic_frontend_error.state);
  EXPECT_FALSE(b.mosfet_software_lock.state);
}

TEST(JbdBmsHwInfoTest, ProtectionBinarySensorsAllActive) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME_ALL_PROTECTIONS);
  EXPECT_TRUE(b.cell_overvoltage_protection.state);
  EXPECT_TRUE(b.cell_undervoltage_protection.state);
  EXPECT_TRUE(b.pack_overvoltage_protection.state);
  EXPECT_TRUE(b.pack_undervoltage_protection.state);
  EXPECT_TRUE(b.charge_overtemperature_protection.state);
  EXPECT_TRUE(b.charge_undertemperature_protection.state);
  EXPECT_TRUE(b.discharge_overtemperature_protection.state);
  EXPECT_TRUE(b.discharge_undertemperature_protection.state);
  EXPECT_TRUE(b.charge_overcurrent_protection.state);
  EXPECT_TRUE(b.discharge_overcurrent_protection.state);
  EXPECT_TRUE(b.short_circuit_protection.state);
  EXPECT_TRUE(b.ic_frontend_error.state);
  EXPECT_TRUE(b.mosfet_software_lock.state);
}

TEST(JbdBmsHwInfoTest, ProtectionBinarySensorsByteOrder) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME_MOSFET_SOFTWARE_LOCK);
  EXPECT_FALSE(b.cell_overvoltage_protection.state);
  EXPECT_FALSE(b.cell_undervoltage_protection.state);
  EXPECT_FALSE(b.pack_overvoltage_protection.state);
  EXPECT_FALSE(b.pack_undervoltage_protection.state);
  EXPECT_FALSE(b.charge_overtemperature_protection.state);
  EXPECT_FALSE(b.charge_undertemperature_protection.state);
  EXPECT_FALSE(b.discharge_overtemperature_protection.state);
  EXPECT_FALSE(b.discharge_undertemperature_protection.state);
  EXPECT_FALSE(b.charge_overcurrent_protection.state);
  EXPECT_FALSE(b.discharge_overcurrent_protection.state);
  EXPECT_FALSE(b.short_circuit_protection.state);
  EXPECT_FALSE(b.ic_frontend_error.state);
  EXPECT_TRUE(b.mosfet_software_lock.state);
}

// ── Balancing cells text sensor ───────────────────────────────────────────────

TEST(JbdBmsHwInfoTest, BalancingCellsEmpty) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  EXPECT_EQ(b.balancing_cells_ts.state, "");
}

TEST(JbdBmsHwInfoTest, BalancingCellsActive) {
  JbdBmsTestBench b;
  b.bms.on_jbd_bms_data(0x03, HWINFO_FRAME_BALANCING_CELLS_1_3);
  EXPECT_EQ(b.balancing_cells_ts.state, "1, 3");
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(JbdBmsTest, NullSensorsDoNotCrash) {
  TestableJbdBms bms;
  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  bms.on_jbd_bms_data(0x04, CELLINFO_FRAME);
  bms.on_jbd_bms_data(0x05, HWVER_FRAME);
  bms.on_jbd_bms_data(0xAA, ERRCOUNT_FRAME);
}

}  // namespace esphome::jbd_bms::testing
