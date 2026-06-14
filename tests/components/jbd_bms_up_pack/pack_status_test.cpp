#include <gtest/gtest.h>
#include "common.h"
#include "frames.h"

namespace esphome::jbd_bms_up_pack::testing {

using esphome::jbd_bms_up::testing::PACK_STATUS_FRAME;
using esphome::jbd_bms_up::testing::PACK_STATUS_FRAME_ADDR2;
using esphome::jbd_bms_up::testing::make_pack_status_balancing;
using esphome::jbd_bms_up::testing::make_pack_status_charging;
using esphome::jbd_bms_up::testing::make_pack_status_with_errors;

// ── Voltage and current — addr=1 ─────────────────────────────────────────────

TEST(PackStatusTest, TotalVoltage) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(total.state, 53.17f, 0.01f);
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
  EXPECT_NEAR(soc.state, 60.95f, 0.01f);
}

TEST(PackStatusTest, Capacity) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cap_rem, nominal;
  pack.set_capacity_remaining_sensor(&cap_rem);
  pack.set_nominal_capacity_sensor(&nominal);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(cap_rem.state, 60.95f, 0.01f);
  EXPECT_NEAR(nominal.state, 100.00f, 0.01f);
}

TEST(PackStatusTest, ChargingCycles) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cycles;
  pack.set_charging_cycles_sensor(&cycles);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(cycles.state, 12.0f);
}

// ── Temperatures ─────────────────────────────────────────────────────────────

TEST(PackStatusTest, MosfetTemperature) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor temp;
  pack.set_mosfet_temperature_sensor(&temp);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(temp.state, 21.0f, 0.1f);
}

TEST(PackStatusTest, AmbientTemperature) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor temp;
  pack.set_ambient_temperature_sensor(&temp);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(temp.state, 22.6f, 0.1f);
}

TEST(PackStatusTest, CellTemperatures) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor t[4];
  for (int i = 0; i < 4; i++)
    pack.set_temperature_sensor(i, &t[i]);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(t[0].state, 20.9f, 0.1f);
  EXPECT_NEAR(t[1].state, 20.4f, 0.1f);
  EXPECT_NEAR(t[2].state, 21.0f, 0.1f);
  EXPECT_NEAR(t[3].state, 21.2f, 0.1f);
}

// ── State of health ───────────────────────────────────────────────────────────

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
  EXPECT_FLOAT_EQ(errors.state, 0x00010001);
  EXPECT_NE(errors_text.state.find("Cell overvoltage"), std::string::npos);
  EXPECT_NE(errors_text.state.find("EEP fault"), std::string::npos);
}

// ── MOSFET status ─────────────────────────────────────────────────────────────

TEST(PackStatusTest, MosfetStatusBothEnabled) {
  TestableJbdBmsUpPack pack;
  binary_sensor::BinarySensor charging, discharging, precharging, heat, fan;
  pack.set_charging_binary_sensor(&charging);
  pack.set_discharging_binary_sensor(&discharging);
  pack.set_precharging_binary_sensor(&precharging);
  pack.set_heat_binary_sensor(&heat);
  pack.set_fan_binary_sensor(&fan);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  // mos=0x0003: bit0=DSG=1, bit1=CHG=1, bit2..4=0
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
  EXPECT_NEAR(max_v.state, 3.331f, 0.001f);
  EXPECT_FLOAT_EQ(max_cell.state, 3.0f);  // 1-indexed cell 3
  EXPECT_NEAR(avg.state, 3.323f, 0.001f);
  EXPECT_NEAR(min_v.state, 3.317f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 8.0f);  // 1-indexed cell 8
  EXPECT_NEAR(delta.state, 0.014f, 0.001f);
}

TEST(PackStatusTest, BatteryStrings) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor strings;
  pack.set_cell_count_sensor(&strings);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(strings.state, 16.0f);
}

TEST(PackStatusTest, CellVoltages) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cells[16];
  for (int i = 0; i < 16; i++)
    pack.set_cell_voltage_sensor(i, &cells[i]);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  // Spot-check known values from capture
  EXPECT_NEAR(cells[0].state, 3.326f, 0.001f);   // cell[0]:  0x0CFE
  EXPECT_NEAR(cells[2].state, 3.331f, 0.001f);   // cell[2]:  0x0D03 ← max
  EXPECT_NEAR(cells[7].state, 3.317f, 0.001f);   // cell[7]:  0x0CF5 ← min
  EXPECT_NEAR(cells[6].state, 3.328f, 0.001f);   // cell[6]:  0x0D00
  EXPECT_NEAR(cells[15].state, 3.322f, 0.001f);  // cell[15]: 0x0CFA
  // All cells within measured range [3.317 V, 3.331 V]
  for (int i = 0; i < 16; i++) {
    EXPECT_GE(cells[i].state, 3.316f) << "cell " << i;
    EXPECT_LE(cells[i].state, 3.332f) << "cell " << i;
  }
}

// ── Rated capacity ────────────────────────────────────────────────────────────

TEST(PackStatusTest, RatedCapacity) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor rated;
  pack.set_rated_capacity_sensor(&rated);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(rated.state, 100.00f, 0.01f);
}

TEST(PackStatusTest, Addr2RatedCapacity) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor rated;
  pack.set_rated_capacity_sensor(&rated);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(rated.state, 100.00f, 0.01f);
}

// ── Temperature aggregates ────────────────────────────────────────────────────

TEST(PackStatusTest, TemperatureAggregates) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor max_t, min_t, avg_t;
  pack.set_max_temperature_sensor(&max_t);
  pack.set_min_temperature_sensor(&min_t);
  pack.set_average_temperature_sensor(&avg_t);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(max_t.state, 21.2f, 0.1f);
  EXPECT_NEAR(min_t.state, 20.4f, 0.1f);
  EXPECT_NEAR(avg_t.state, 20.8f, 0.1f);
}

TEST(PackStatusTest, Addr2TemperatureAggregates) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor max_t, min_t, avg_t;
  pack.set_max_temperature_sensor(&max_t);
  pack.set_min_temperature_sensor(&min_t);
  pack.set_average_temperature_sensor(&avg_t);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(max_t.state, 22.0f, 0.1f);
  EXPECT_NEAR(min_t.state, 21.3f, 0.1f);
  EXPECT_NEAR(avg_t.state, 21.6f, 0.1f);
}

// ── Charge/discharge limits ───────────────────────────────────────────────────

TEST(PackStatusTest, ChargeDischargeLimits) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cvl, ccl, dvl, dcl;
  pack.set_charge_voltage_limit_sensor(&cvl);
  pack.set_charge_current_limit_sensor(&ccl);
  pack.set_discharge_voltage_limit_sensor(&dvl);
  pack.set_discharge_current_limit_sensor(&dcl);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_NEAR(cvl.state, 58.4f, 0.1f);
  EXPECT_NEAR(ccl.state, 200.0f, 0.1f);
  EXPECT_NEAR(dvl.state, 44.8f, 0.1f);
  EXPECT_NEAR(dcl.state, 200.0f, 0.1f);
}

TEST(PackStatusTest, Addr2ChargeDischargeLimits) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cvl, ccl, dvl, dcl;
  pack.set_charge_voltage_limit_sensor(&cvl);
  pack.set_charge_current_limit_sensor(&ccl);
  pack.set_discharge_voltage_limit_sensor(&dvl);
  pack.set_discharge_current_limit_sensor(&dcl);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(cvl.state, 58.4f, 0.1f);
  EXPECT_NEAR(ccl.state, 100.0f, 0.1f);
  EXPECT_NEAR(dvl.state, 44.8f, 0.1f);
  EXPECT_NEAR(dcl.state, 100.0f, 0.1f);
}

// ── Balancing ─────────────────────────────────────────────────────────────────

TEST(PackStatusTest, BalancingNotActive) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor bitmask;
  binary_sensor::BinarySensor active;
  pack.set_balancing_bitmask_sensor(&bitmask);
  pack.set_balancing_binary_sensor(&active);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_FLOAT_EQ(bitmask.state, 0.0f);
  EXPECT_FALSE(active.state);
}

TEST(PackStatusTest, BalancingActive) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor bitmask;
  binary_sensor::BinarySensor active;
  pack.set_balancing_bitmask_sensor(&bitmask);
  pack.set_balancing_binary_sensor(&active);
  pack.on_pack_status_(make_pack_status_balancing());
  EXPECT_FLOAT_EQ(bitmask.state, 3.0f);  // bits 0+1 → cells 1 and 2
  EXPECT_TRUE(active.state);
}

// ── Firmware version ──────────────────────────────────────────────────────────

TEST(PackStatusTest, FirmwareVersion) {
  TestableJbdBmsUpPack pack;
  text_sensor::TextSensor fw;
  pack.set_firmware_version_text_sensor(&fw);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_EQ(fw.state, "12.4");
}

TEST(PackStatusTest, Addr2FirmwareVersion) {
  TestableJbdBmsUpPack pack;
  text_sensor::TextSensor fw;
  pack.set_firmware_version_text_sensor(&fw);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_EQ(fw.state, "0.4");
}

// ── Device model ──────────────────────────────────────────────────────────────

TEST(PackStatusTest, DeviceModel) {
  TestableJbdBmsUpPack pack;
  text_sensor::TextSensor model;
  pack.set_device_model_text_sensor(&model);
  pack.on_pack_status_(PACK_STATUS_FRAME);
  EXPECT_EQ(model.state.substr(0, 11), "JBD48100000");
}

// ── Second device (addr=2) ────────────────────────────────────────────────────

TEST(PackStatusTest, Addr2TotalVoltage) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(total.state, 53.21f, 0.01f);
}

TEST(PackStatusTest, Addr2StateOfCharge) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor soc;
  pack.set_state_of_charge_sensor(&soc);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(soc.state, 73.00f, 0.01f);
}

TEST(PackStatusTest, Addr2ChargingCycles) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor cycles;
  pack.set_charging_cycles_sensor(&cycles);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_FLOAT_EQ(cycles.state, 2.0f);
}

TEST(PackStatusTest, Addr2CellVoltageStats) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor min_v, max_v, min_cell, max_cell, avg, delta;
  pack.set_min_cell_voltage_sensor(&min_v);
  pack.set_max_cell_voltage_sensor(&max_v);
  pack.set_min_voltage_cell_sensor(&min_cell);
  pack.set_max_voltage_cell_sensor(&max_cell);
  pack.set_average_cell_voltage_sensor(&avg);
  pack.set_delta_cell_voltage_sensor(&delta);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(max_v.state, 3.329f, 0.001f);
  EXPECT_FLOAT_EQ(max_cell.state, 4.0f);  // 1-indexed cell 4
  EXPECT_NEAR(avg.state, 3.325f, 0.001f);
  EXPECT_NEAR(min_v.state, 3.322f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 1.0f);  // 1-indexed cell 1
  EXPECT_NEAR(delta.state, 0.007f, 0.001f);
}

TEST(PackStatusTest, Addr2CellTemperatures) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor t[4];
  for (int i = 0; i < 4; i++)
    pack.set_temperature_sensor(i, &t[i]);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(t[0].state, 21.4f, 0.1f);
  EXPECT_NEAR(t[1].state, 21.3f, 0.1f);
  EXPECT_NEAR(t[2].state, 22.0f, 0.1f);
  EXPECT_NEAR(t[3].state, 21.8f, 0.1f);
}

// ── Robustness ────────────────────────────────────────────────────────────────

TEST(PackStatusTest, NullSensorsDoNotCrash) {
  TestableJbdBmsUpPack pack;
  pack.on_pack_status_(PACK_STATUS_FRAME);
  pack.on_pack_status_(PACK_STATUS_FRAME_ADDR2);
  pack.on_pack_status_(make_pack_status_charging());
  pack.on_pack_status_(make_pack_status_with_errors());
  pack.on_pack_status_(make_pack_status_balancing());
}

TEST(PackStatusTest, ShortFrameDoesNotCrash) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  pack.on_pack_status_({0x14, 0x8f, 0x00, 0x00});
}

// ── on_jbd_bms_up_data dispatch ───────────────────────────────────────────────

TEST(PackStatusTest, DispatchPackStatus) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  pack.on_jbd_bms_up_data(0x78, 0x1000, PACK_STATUS_FRAME);
  EXPECT_NEAR(total.state, 53.17f, 0.01f);
}

TEST(PackStatusTest, DispatchPackStatusAddr2) {
  TestableJbdBmsUpPack pack;
  sensor::Sensor total;
  pack.set_total_voltage_sensor(&total);
  pack.on_jbd_bms_up_data(0x78, 0x1000, PACK_STATUS_FRAME_ADDR2);
  EXPECT_NEAR(total.state, 53.21f, 0.01f);
}

TEST(PackStatusTest, UnknownFunctionDoesNotCrash) {
  TestableJbdBmsUpPack pack;
  pack.on_jbd_bms_up_data(0x78, 0x1C00, {0x00, 0x01, 0x02});
  pack.on_jbd_bms_up_data(0x45, 0x0000, {0x00, 0x01, 0x02});
}

}  // namespace esphome::jbd_bms_up_pack::testing
