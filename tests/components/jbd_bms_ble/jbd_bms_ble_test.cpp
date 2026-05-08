#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_ble::testing {

// ── Total voltage ─────────────────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, TotalVoltage) {
  TestableJbdBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_NEAR(total.state, 15.60f, 0.01f);
}

// ── Current and power ─────────────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, CurrentAndPower) {
  TestableJbdBmsBle bms;
  sensor::Sensor current, power, charging_power, discharging_power;
  bms.set_current_sensor(&current);
  bms.set_power_sensor(&power);
  bms.set_charging_power_sensor(&charging_power);
  bms.set_discharging_power_sensor(&discharging_power);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_NEAR(current.state, 0.0f, 0.001f);
  EXPECT_NEAR(power.state, 0.0f, 0.01f);
  EXPECT_NEAR(charging_power.state, 0.0f, 0.01f);
  EXPECT_NEAR(discharging_power.state, 0.0f, 0.01f);
}

// ── Capacity ──────────────────────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, Capacity) {
  TestableJbdBmsBle bms;
  sensor::Sensor cap_rem, nominal, cycles;
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_nominal_capacity_sensor(&nominal);
  bms.set_charging_cycles_sensor(&cycles);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_NEAR(cap_rem.state, 4.98f, 0.01f);
  EXPECT_NEAR(nominal.state, 5.00f, 0.01f);
  EXPECT_FLOAT_EQ(cycles.state, 0.0f);
}

// ── State of charge ───────────────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, StateOfCharge) {
  TestableJbdBmsBle bms;
  sensor::Sensor soc;
  bms.set_state_of_charge_sensor(&soc);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_FLOAT_EQ(soc.state, 100.0f);
}

// ── Software version ──────────────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, SoftwareVersion) {
  TestableJbdBmsBle bms;
  sensor::Sensor ver;
  bms.set_software_version_sensor(&ver);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_NEAR(ver.state, 8.0f, 0.01f);
}

// ── Battery strings and temperature sensors ───────────────────────────────────

TEST(JbdBmsBleHwInfoTest, BatteryStringsAndTempSensors) {
  TestableJbdBmsBle bms;
  sensor::Sensor strings, temp_sensors;
  bms.set_battery_strings_sensor(&strings);
  bms.set_temperature_sensors_sensor(&temp_sensors);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_FLOAT_EQ(strings.state, 4.0f);
  EXPECT_FLOAT_EQ(temp_sensors.state, 3.0f);
}

// ── Temperatures ──────────────────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, Temperatures) {
  TestableJbdBmsBle bms;
  sensor::Sensor t[3];
  for (int i = 0; i < 3; i++)
    bms.set_temperature_sensor(i, &t[i]);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_NEAR(t[0].state, 22.4f, 0.05f);
  EXPECT_NEAR(t[1].state, 22.3f, 0.05f);
  EXPECT_NEAR(t[2].state, 21.7f, 0.05f);
}

// ── Balancer and error bitmasks ───────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, Bitmasks) {
  TestableJbdBmsBle bms;
  sensor::Sensor bal_bitmask, err_bitmask, op_bitmask;
  bms.set_balancer_status_bitmask_sensor(&bal_bitmask);
  bms.set_errors_bitmask_sensor(&err_bitmask);
  bms.set_operation_status_bitmask_sensor(&op_bitmask);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_FLOAT_EQ(bal_bitmask.state, 0.0f);
  EXPECT_FLOAT_EQ(err_bitmask.state, 0.0f);
  EXPECT_FLOAT_EQ(op_bitmask.state, 3.0f);
}

// ── Binary sensors and switches ───────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, BinarySensorsAndSwitches) {
  TestableJbdBmsBle bms;
  binary_sensor::BinarySensor charging_bs, discharging_bs, balancing_bs;
  TestableSwitch charging_sw, discharging_sw;
  bms.set_charging_binary_sensor(&charging_bs);
  bms.set_discharging_binary_sensor(&discharging_bs);
  bms.set_balancing_binary_sensor(&balancing_bs);
  bms.set_charging_switch(&charging_sw);
  bms.set_discharging_switch(&discharging_sw);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_TRUE(charging_bs.state);
  EXPECT_TRUE(discharging_bs.state);
  EXPECT_FALSE(balancing_bs.state);
  EXPECT_TRUE(charging_sw.state);
  EXPECT_TRUE(discharging_sw.state);
}

// ── Operation status text ─────────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, OperationStatusText) {
  TestableJbdBmsBle bms;
  text_sensor::TextSensor op_status;
  bms.set_operation_status_text_sensor(&op_status);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_NE(op_status.state.find("Charging"), std::string::npos);
  EXPECT_NE(op_status.state.find("Discharging"), std::string::npos);
}

// ── Errors text (no errors) ───────────────────────────────────────────────────

TEST(JbdBmsBleHwInfoTest, ErrorsTextEmpty) {
  TestableJbdBmsBle bms;
  text_sensor::TextSensor errors;
  bms.set_errors_text_sensor(&errors);

  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);

  EXPECT_EQ(errors.state, "");
}

// ── Cell voltages ─────────────────────────────────────────────────────────────

TEST(JbdBmsBleCellInfoTest, CellVoltages) {
  TestableJbdBmsBle bms;
  sensor::Sensor cells[4];
  for (int i = 0; i < 4; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.on_jbd_bms_data(0x04, CELLINFO_FRAME);

  EXPECT_NEAR(cells[0].state, 3.909f, 0.001f);
  EXPECT_NEAR(cells[1].state, 3.901f, 0.001f);
  EXPECT_NEAR(cells[2].state, 3.895f, 0.001f);
  EXPECT_NEAR(cells[3].state, 3.901f, 0.001f);
}

// ── Cell voltage statistics ───────────────────────────────────────────────────

TEST(JbdBmsBleCellInfoTest, CellVoltageStats) {
  TestableJbdBmsBle bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.on_jbd_bms_data(0x04, CELLINFO_FRAME);

  EXPECT_NEAR(min_v.state, 3.895f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.909f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 3.0f);
  EXPECT_FLOAT_EQ(max_cell.state, 1.0f);
  EXPECT_NEAR(delta.state, 0.014f, 0.001f);
  EXPECT_NEAR(avg.state, 3.9015f, 0.001f);
}

// ── Device model ──────────────────────────────────────────────────────────────

TEST(JbdBmsBleHwVerTest, DeviceModel) {
  TestableJbdBmsBle bms;
  text_sensor::TextSensor model;
  bms.set_device_model_text_sensor(&model);

  bms.on_jbd_bms_data(0x05, HWVER_FRAME);

  EXPECT_EQ(model.state, "JBD-SP04S034-L4S-200A-B-U");
}

// ── Error counts ──────────────────────────────────────────────────────────────

TEST(JbdBmsBleErrorCountsTest, ErrorCounts) {
  TestableJbdBmsBle bms;
  sensor::Sensor sc, co, doc, cov, cuv, cot, cut, dot, dut, bov, buv;
  bms.set_short_circuit_error_count_sensor(&sc);
  bms.set_charge_overcurrent_error_count_sensor(&co);
  bms.set_discharge_overcurrent_error_count_sensor(&doc);
  bms.set_cell_overvoltage_error_count_sensor(&cov);
  bms.set_cell_undervoltage_error_count_sensor(&cuv);
  bms.set_charge_overtemperature_error_count_sensor(&cot);
  bms.set_charge_undertemperature_error_count_sensor(&cut);
  bms.set_discharge_overtemperature_error_count_sensor(&dot);
  bms.set_discharge_undertemperature_error_count_sensor(&dut);
  bms.set_battery_overvoltage_error_count_sensor(&bov);
  bms.set_battery_undervoltage_error_count_sensor(&buv);

  bms.on_jbd_bms_data(0xAA, ERRCOUNT_FRAME);

  EXPECT_FLOAT_EQ(sc.state, 0.0f);
  EXPECT_FLOAT_EQ(co.state, 0.0f);
  EXPECT_FLOAT_EQ(doc.state, 0.0f);
  EXPECT_FLOAT_EQ(cov.state, 122.0f);
  EXPECT_FLOAT_EQ(cuv.state, 2.0f);
  EXPECT_FLOAT_EQ(cot.state, 0.0f);
  EXPECT_FLOAT_EQ(cut.state, 0.0f);
  EXPECT_FLOAT_EQ(dot.state, 0.0f);
  EXPECT_FLOAT_EQ(dut.state, 0.0f);
  EXPECT_FLOAT_EQ(bov.state, 0.0f);
  EXPECT_FLOAT_EQ(buv.state, 0.0f);
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(JbdBmsBleTest, NullSensorsDoNotCrash) {
  TestableJbdBmsBle bms;
  bms.on_jbd_bms_data(0x03, HWINFO_FRAME);
  bms.on_jbd_bms_data(0x04, CELLINFO_FRAME);
  bms.on_jbd_bms_data(0x05, HWVER_FRAME);
  bms.on_jbd_bms_data(0xAA, ERRCOUNT_FRAME);
}

}  // namespace esphome::jbd_bms_ble::testing
