#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/jbd_bms/jbd_bms.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome::jbd_bms::testing {

class TestableJbdBms : public JbdBms {
 public:
  void update() override {}
  void send_command(uint8_t command, uint8_t address, const uint8_t *data, uint8_t data_len) override {}

  void set_mosfet_status(uint8_t status) { this->mosfet_status_ = status; }
  bool write_register(uint8_t address, uint16_t value) override {
    last_write_address = address;
    last_write_value = value;
    return true;
  }
  using JbdBms::build_frame_;
  uint8_t last_write_address{0};
  uint16_t last_write_value{0};
};

class TestableSwitch : public switch_::Switch {
 public:
  void write_state(bool state) override { this->publish_state(state); }
};

// Pre-wired fixture with all entities attached to a single TestableJbdBms.
// Stack-allocate one per test that needs parsed output.
struct JbdBmsTestBench {
  TestableJbdBms bms;

  sensor::Sensor total_voltage, current, power, charging_power, discharging_power;
  sensor::Sensor state_of_charge, nominal_capacity, charging_cycles, capacity_remaining, battery_cycle_capacity;
  sensor::Sensor min_cell_voltage, max_cell_voltage, min_voltage_cell, max_voltage_cell;
  sensor::Sensor delta_cell_voltage, average_cell_voltage;
  sensor::Sensor operation_status_bitmask, errors_bitmask, balancer_status_bitmask;
  sensor::Sensor battery_strings, temperature_sensors_count, software_version;
  sensor::Sensor short_circuit_error_count, charge_overcurrent_error_count, discharge_overcurrent_error_count;
  sensor::Sensor cell_overvoltage_error_count, cell_undervoltage_error_count;
  sensor::Sensor charge_overtemperature_error_count, charge_undertemperature_error_count;
  sensor::Sensor discharge_overtemperature_error_count, discharge_undertemperature_error_count;
  sensor::Sensor battery_overvoltage_error_count, battery_undervoltage_error_count;
  sensor::Sensor cells[32];
  sensor::Sensor temps[6];

  binary_sensor::BinarySensor charging_bs, discharging_bs, balancing_bs, online_status_bs;
  binary_sensor::BinarySensor cell_overvoltage_protection, cell_undervoltage_protection;
  binary_sensor::BinarySensor pack_overvoltage_protection, pack_undervoltage_protection;
  binary_sensor::BinarySensor charge_overtemperature_protection, charge_undertemperature_protection;
  binary_sensor::BinarySensor discharge_overtemperature_protection, discharge_undertemperature_protection;
  binary_sensor::BinarySensor charge_overcurrent_protection, discharge_overcurrent_protection;
  binary_sensor::BinarySensor short_circuit_protection, ic_frontend_error, mosfet_software_lock;

  text_sensor::TextSensor errors_ts, operation_status_ts, device_model_ts, balancing_cells_ts;

  TestableSwitch charging_sw, discharging_sw;

  JbdBmsTestBench() {
    bms.set_total_voltage_sensor(&total_voltage);
    bms.set_current_sensor(&current);
    bms.set_power_sensor(&power);
    bms.set_charging_power_sensor(&charging_power);
    bms.set_discharging_power_sensor(&discharging_power);
    bms.set_state_of_charge_sensor(&state_of_charge);
    bms.set_nominal_capacity_sensor(&nominal_capacity);
    bms.set_charging_cycles_sensor(&charging_cycles);
    bms.set_capacity_remaining_sensor(&capacity_remaining);
    bms.set_battery_cycle_capacity_sensor(&battery_cycle_capacity);
    bms.set_min_cell_voltage_sensor(&min_cell_voltage);
    bms.set_max_cell_voltage_sensor(&max_cell_voltage);
    bms.set_min_voltage_cell_sensor(&min_voltage_cell);
    bms.set_max_voltage_cell_sensor(&max_voltage_cell);
    bms.set_delta_cell_voltage_sensor(&delta_cell_voltage);
    bms.set_average_cell_voltage_sensor(&average_cell_voltage);
    bms.set_operation_status_bitmask_sensor(&operation_status_bitmask);
    bms.set_errors_bitmask_sensor(&errors_bitmask);
    bms.set_balancer_status_bitmask_sensor(&balancer_status_bitmask);
    bms.set_battery_strings_sensor(&battery_strings);
    bms.set_temperature_sensors_sensor(&temperature_sensors_count);
    bms.set_software_version_sensor(&software_version);
    bms.set_short_circuit_error_count_sensor(&short_circuit_error_count);
    bms.set_charge_overcurrent_error_count_sensor(&charge_overcurrent_error_count);
    bms.set_discharge_overcurrent_error_count_sensor(&discharge_overcurrent_error_count);
    bms.set_cell_overvoltage_error_count_sensor(&cell_overvoltage_error_count);
    bms.set_cell_undervoltage_error_count_sensor(&cell_undervoltage_error_count);
    bms.set_charge_overtemperature_error_count_sensor(&charge_overtemperature_error_count);
    bms.set_charge_undertemperature_error_count_sensor(&charge_undertemperature_error_count);
    bms.set_discharge_overtemperature_error_count_sensor(&discharge_overtemperature_error_count);
    bms.set_discharge_undertemperature_error_count_sensor(&discharge_undertemperature_error_count);
    bms.set_battery_overvoltage_error_count_sensor(&battery_overvoltage_error_count);
    bms.set_battery_undervoltage_error_count_sensor(&battery_undervoltage_error_count);
    for (int i = 0; i < 32; i++)
      bms.set_cell_voltage_sensor(i, &cells[i]);
    for (int i = 0; i < 6; i++)
      bms.set_temperature_sensor(i, &temps[i]);
    bms.set_charging_binary_sensor(&charging_bs);
    bms.set_discharging_binary_sensor(&discharging_bs);
    bms.set_balancing_binary_sensor(&balancing_bs);
    bms.set_online_status_binary_sensor(&online_status_bs);
    bms.set_cell_overvoltage_protection_binary_sensor(&cell_overvoltage_protection);
    bms.set_cell_undervoltage_protection_binary_sensor(&cell_undervoltage_protection);
    bms.set_pack_overvoltage_protection_binary_sensor(&pack_overvoltage_protection);
    bms.set_pack_undervoltage_protection_binary_sensor(&pack_undervoltage_protection);
    bms.set_charge_overtemperature_protection_binary_sensor(&charge_overtemperature_protection);
    bms.set_charge_undertemperature_protection_binary_sensor(&charge_undertemperature_protection);
    bms.set_discharge_overtemperature_protection_binary_sensor(&discharge_overtemperature_protection);
    bms.set_discharge_undertemperature_protection_binary_sensor(&discharge_undertemperature_protection);
    bms.set_charge_overcurrent_protection_binary_sensor(&charge_overcurrent_protection);
    bms.set_discharge_overcurrent_protection_binary_sensor(&discharge_overcurrent_protection);
    bms.set_short_circuit_protection_binary_sensor(&short_circuit_protection);
    bms.set_ic_frontend_error_binary_sensor(&ic_frontend_error);
    bms.set_mosfet_software_lock_binary_sensor(&mosfet_software_lock);
    bms.set_errors_text_sensor(&errors_ts);
    bms.set_operation_status_text_sensor(&operation_status_ts);
    bms.set_device_model_text_sensor(&device_model_ts);
    bms.set_balancing_cells_text_sensor(&balancing_cells_ts);
    bms.set_charging_switch(&charging_sw);
    bms.set_discharging_switch(&discharging_sw);
  }
};

}  // namespace esphome::jbd_bms::testing
