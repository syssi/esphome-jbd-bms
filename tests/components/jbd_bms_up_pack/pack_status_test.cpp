#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_up_pack::testing {

using esphome::jbd_bms_up::testing::PACK_STATUS_FRAME;
using esphome::jbd_bms_up::testing::make_pack_status_charging;
using esphome::jbd_bms_up::testing::make_pack_status_with_errors;

// ── Voltage and current ───────────────────────────────────────────────────────

TEST(PackStatusTest, TotalVoltage) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(total.state, 52.90f, 0.01f);
}

TEST(PackStatusTest, CurrentZero) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor current;
  pack.set_current_sensor(&current);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(current.state, 0.0f, 0.01f);
}

TEST(PackStatusTest, CurrentAndPowerCharging) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor current, power, charging_power, discharging_power;
  pack.set_current_sensor(&current);
  pack.set_power_sensor(&power);
  pack.set_charging_power_sensor(&charging_power);
  pack.set_discharging_power_sensor(&discharging_power);

  pack.on_pack_status_(make_pack_status_charging());

  EXPECT_NEAR(current.state, 50.0f, 0.01f);
  EXPECT_GT(power.state, 0.0f);
  EXPECT_GT(charging_power.state, 0.0f);
  EXPECT_NEAR(discharging_power.state, 0.0f, 0.01f);
}

// ── State of charge and capacity ─────────────────────────────────────────────

TEST(PackStatusTest, StateOfCharge) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor soc;
  pack.set_state_of_charge_sensor(&soc);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(soc.state, 45.19f, 0.01f);
}

TEST(PackStatusTest, Capacity) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cap_rem, nominal;
  pack.set_capacity_remaining_sensor(&cap_rem);
  pack.set_nominal_capacity_sensor(&nominal);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(cap_rem.state, 45.19f, 0.01f);
  EXPECT_NEAR(nominal.state, 100.00f, 0.01f);
}

TEST(PackStatusTest, ChargingCycles) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cycles;
  pack.set_charging_cycles_sensor(&cycles);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(cycles.state, 12.0f);
}

// ── Temperatures (UP-only sensors) ───────────────────────────────────────────

TEST(PackStatusTest, MosfetTemperature) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor temp;
  pack.set_mosfet_temperature_sensor(&temp);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(temp.state, 25.6f, 0.1f);
}

TEST(PackStatusTest, AmbientTemperature) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor temp;
  pack.set_ambient_temperature_sensor(&temp);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(temp.state, 27.4f, 0.1f);
}

TEST(PackStatusTest, CellTemperatures) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor t[4];
  for (int i = 0; i < 4; i++)
    pack.set_temperature_sensor(i, &t[i]);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(t[0].state, 26.0f, 0.1f);
  EXPECT_NEAR(t[1].state, 25.4f, 0.1f);
  EXPECT_NEAR(t[2].state, 26.2f, 0.1f);
  EXPECT_NEAR(t[3].state, 26.4f, 0.1f);
}

// ── State of health (UP-only) ─────────────────────────────────────────────────

TEST(PackStatusTest, StateOfHealth) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor soh;
  pack.set_state_of_health_sensor(&soh);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(soh.state, 100.0f);
}

// ── Operation status ──────────────────────────────────────────────────────────

TEST(PackStatusTest, OperationStatusIdle) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor bitmask;
  text_sensor::TextSensor text;
  pack.set_operation_status_bitmask_sensor(&bitmask);
  pack.set_operation_status_text_sensor(&text);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(bitmask.state, 0.0f);
  EXPECT_EQ(text.state, "Idle");
}

TEST(PackStatusTest, OperationStatusCharging) {
  TestableJbdBmsUpPack pack;
  text_sensor::TextSensor text;
  pack.set_operation_status_text_sensor(&text);
  pack.on_pack_status_(make_pack_status_charging());
  EXPECT_EQ(text.state, "Charging");
}

// ── Bitmasks ──────────────────────────────────────────────────────────────────

TEST(PackStatusTest, BitmasksZero) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor protect, errors;
  pack.set_protect_bitmask_sensor(&protect);
  pack.set_errors_bitmask_sensor(&errors);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(protect.state, 0.0f);
  EXPECT_FLOAT_EQ(errors.state, 0.0f);
}

TEST(PackStatusTest, ErrorsBitmaskDecoded) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor errors;
  text_sensor::TextSensor errors_text;
  pack.set_errors_bitmask_sensor(&errors);
  pack.set_errors_text_sensor(&errors_text);
  pack.on_pack_status_(make_pack_status_with_errors());
  // errors_bitmask = 0x00010001: bit0=cell_overvoltage, bit16=eep_fault
  EXPECT_FLOAT_EQ(errors.state, 0x00010001);
  EXPECT_NE(errors_text.state.find("Cell overvoltage"), std::string::npos);
  EXPECT_NE(errors_text.state.find("EEP fault"), std::string::npos);
}

// ── MOSFET status and binary sensors ─────────────────────────────────────────

TEST(PackStatusTest, MosfetStatusBothEnabled) {
  TestableJbdBmsUpPack pack;
  binary_sensor::BinarySensor charging, discharging, precharging, heat, fan;
  pack.set_charging_binary_sensor(&charging);
  pack.set_discharging_binary_sensor(&discharging);
  pack.set_precharging_binary_sensor(&precharging);
  pack.set_heat_binary_sensor(&heat);
  pack.set_fan_binary_sensor(&fan);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  // MOSFET status 0x0003: bit0=DSG=1, bit1=CHG=1, bit2=precharge=0
  EXPECT_TRUE(discharging.state);
  EXPECT_TRUE(charging.state);
  EXPECT_FALSE(precharging.state);
  EXPECT_FALSE(heat.state);
  EXPECT_FALSE(fan.state);
}

// ── Cell voltage statistics ───────────────────────────────────────────────────

TEST(PackStatusTest, CellVoltageStats) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor min_v, max_v, min_cell, max_cell, avg, delta;
  pack.set_min_cell_voltage_sensor(&min_v);
  pack.set_max_cell_voltage_sensor(&max_v);
  pack.set_min_voltage_cell_sensor(&min_cell);
  pack.set_max_voltage_cell_sensor(&max_cell);
  pack.set_average_cell_voltage_sensor(&avg);
  pack.set_delta_cell_voltage_sensor(&delta);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(max_v.state, 3.307f, 0.001f);
  EXPECT_FLOAT_EQ(max_cell.state, 1.0f);
  EXPECT_NEAR(avg.state, 3.306f, 0.001f);
  EXPECT_NEAR(min_v.state, 3.306f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 2.0f);
  EXPECT_NEAR(delta.state, 0.001f, 0.001f);
}

// ── Individual cell voltages ──────────────────────────────────────────────────

TEST(PackStatusTest, BatteryStrings) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor strings;
  pack.set_battery_strings_sensor(&strings);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(strings.state, 16.0f);
}

TEST(PackStatusTest, CellVoltages) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cells[16];
  for (int i = 0; i < 16; i++)
    pack.set_cell_voltage_sensor(i, &cells[i]);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  // All cells between 3.306 and 3.308 V (3.308 accounts for float precision of 0.001 scaling)
  for (int i = 0; i < 16; i++) {
    EXPECT_GE(cells[i].state, 3.306f) << "cell " << i;
    EXPECT_LT(cells[i].state, 3.308f) << "cell " << i;
  }
  // Spot-check known values (0-indexed cells)
  EXPECT_NEAR(cells[0].state, 3.307f, 0.001f);   // cell[0]:  0x0CEB
  EXPECT_NEAR(cells[2].state, 3.306f, 0.001f);   // cell[2]:  0x0CEA
  EXPECT_NEAR(cells[8].state, 3.306f, 0.001f);   // cell[8]:  0x0CEA
  EXPECT_NEAR(cells[13].state, 3.306f, 0.001f);  // cell[13]: 0x0CEA
  EXPECT_NEAR(cells[15].state, 3.307f, 0.001f);  // cell[15]: 0x0CEB
}

// ── Device model ──────────────────────────────────────────────────────────────

TEST(PackStatusTest, DeviceModel) {
  TestableJbdBmsUpPack pack;
  text_sensor::TextSensor model;
  pack.set_device_model_text_sensor(&model);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_EQ(model.state.substr(0, 10), "JBD4810000");
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(PackStatusTest, NullSensorsDoNotCrash) {
  TestableJbdBmsUpPack pack;
  pack.on_pack_status_(PACK_STATUS_FRAME);
  pack.on_pack_status_(make_pack_status_charging());
  pack.on_pack_status_(make_pack_status_with_errors());
}

TEST(PackStatusTest, ShortFrameDoesNotCrash) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  // Frame too short: should be rejected gracefully
  pack.on_pack_status_({0x14, 0x8f, 0x00, 0x00});
  // sensor stays at default (0), no crash
}

// ── on_jbd_bms_up_data dispatch ───────────────────────────────────────────────

TEST(PackStatusTest, DispatchPackStatus) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  // 0x78 + 0x1000 → pack status
  pack.on_jbd_bms_up_data(0x78, 0x1000, PACK_STATUS_FRAME);
  EXPECT_NEAR(total.state, 52.90f, 0.01f);
}

TEST(PackStatusTest, UnknownFunctionDoesNotCrash) {
  TestableJbdBmsUpPack pack;
  pack.on_jbd_bms_up_data(0x78, 0x1C00, {0x00, 0x01, 0x02});
  pack.on_jbd_bms_up_data(0x45, 0x0000, {0x00, 0x01, 0x02});
}

// ── @TODO: missing response decode tests ─────────────────────────────────────
//
// @TODO Pack Parameters decode (0x1C00 response, 136 bytes):
//   Contains: balance_voltage, charge/discharge thresholds, over/under voltage
//   limits, temperature thresholds, BMS code string, firmware date.
//   Needs: on_pack_parameters_() + corresponding sensor entities.
//
// @TODO Alternate Pack Status decode (FC=0x45 response, 124 bytes):
//   Contains: per-cell voltages, temperatures, capacity percentages, status.
//   Different frame layout from 0x1000 response.
//   Needs: on_alt_pack_status_() handler + FC=0x45 dispatch in on_jbd_bms_up_data.
//
// @TODO MOS status with precharging/heat/fan bits active:
//   Synthetic frame with MOSFET status 0x001C (bits 2,3,4 set).
//   Verifies precharging=true, heat=true, fan=true.
//
// @TODO Discharging current (negative current):
//   Synthetic frame with current = 295000 → (295000-300000)*0.01 = -50.00 A.
//   Verifies: current=-50A, discharging_power>0, charging_power=0.
//
// @TODO Protect bitmask with all 27 flags set:
//   Verifies bitmask_to_string_ covers all PROTECT[] entries.

}  // namespace esphome::jbd_bms_up_pack::testing
