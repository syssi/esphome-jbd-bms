#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/jbd_bms_up/jbd_bms_up.h"

namespace esphome::jbd_bms_up_pack {

class JbdBmsUpPack : public PollingComponent, public jbd_bms_up::JbdBmsUpDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void on_jbd_bms_up_data(uint8_t function, uint16_t start_addr, const std::vector<uint8_t> &data) override;

  void set_total_voltage_sensor(sensor::Sensor *total_voltage_sensor) { total_voltage_sensor_ = total_voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_charging_power_sensor(sensor::Sensor *charging_power_sensor) {
    charging_power_sensor_ = charging_power_sensor;
  }
  void set_discharging_power_sensor(sensor::Sensor *discharging_power_sensor) {
    discharging_power_sensor_ = discharging_power_sensor;
  }
  void set_state_of_charge_sensor(sensor::Sensor *state_of_charge_sensor) {
    state_of_charge_sensor_ = state_of_charge_sensor;
  }
  void set_capacity_remaining_sensor(sensor::Sensor *capacity_remaining_sensor) {
    capacity_remaining_sensor_ = capacity_remaining_sensor;
  }
  void set_nominal_capacity_sensor(sensor::Sensor *nominal_capacity_sensor) {
    nominal_capacity_sensor_ = nominal_capacity_sensor;
  }
  void set_mosfet_temperature_sensor(sensor::Sensor *mosfet_temperature_sensor) {
    mosfet_temperature_sensor_ = mosfet_temperature_sensor;
  }
  void set_ambient_temperature_sensor(sensor::Sensor *ambient_temperature_sensor) {
    ambient_temperature_sensor_ = ambient_temperature_sensor;
  }
  void set_state_of_health_sensor(sensor::Sensor *state_of_health_sensor) {
    state_of_health_sensor_ = state_of_health_sensor;
  }
  void set_operation_status_bitmask_sensor(sensor::Sensor *operation_status_bitmask_sensor) {
    operation_status_bitmask_sensor_ = operation_status_bitmask_sensor;
  }
  void set_errors_bitmask_sensor(sensor::Sensor *errors_bitmask_sensor) {
    errors_bitmask_sensor_ = errors_bitmask_sensor;
  }
  void set_protect_bitmask_sensor(sensor::Sensor *protect_bitmask_sensor) {
    protect_bitmask_sensor_ = protect_bitmask_sensor;
  }
  void set_charging_cycles_sensor(sensor::Sensor *charging_cycles_sensor) {
    charging_cycles_sensor_ = charging_cycles_sensor;
  }
  void set_min_cell_voltage_sensor(sensor::Sensor *min_cell_voltage_sensor) {
    min_cell_voltage_sensor_ = min_cell_voltage_sensor;
  }
  void set_max_cell_voltage_sensor(sensor::Sensor *max_cell_voltage_sensor) {
    max_cell_voltage_sensor_ = max_cell_voltage_sensor;
  }
  void set_min_voltage_cell_sensor(sensor::Sensor *min_voltage_cell_sensor) {
    min_voltage_cell_sensor_ = min_voltage_cell_sensor;
  }
  void set_max_voltage_cell_sensor(sensor::Sensor *max_voltage_cell_sensor) {
    max_voltage_cell_sensor_ = max_voltage_cell_sensor;
  }
  void set_delta_cell_voltage_sensor(sensor::Sensor *delta_cell_voltage_sensor) {
    delta_cell_voltage_sensor_ = delta_cell_voltage_sensor;
  }
  void set_average_cell_voltage_sensor(sensor::Sensor *average_cell_voltage_sensor) {
    average_cell_voltage_sensor_ = average_cell_voltage_sensor;
  }
  void set_cell_count_sensor(sensor::Sensor *cell_count_sensor) { cell_count_sensor_ = cell_count_sensor; }

  void set_cell_voltage_sensor(uint8_t cell, sensor::Sensor *sensor) { cell_voltage_sensors_[cell] = sensor; }
  void set_temperature_sensor(uint8_t index, sensor::Sensor *sensor) { temperature_sensors_[index] = sensor; }

  void set_charging_binary_sensor(binary_sensor::BinarySensor *charging_binary_sensor) {
    charging_binary_sensor_ = charging_binary_sensor;
  }
  void set_discharging_binary_sensor(binary_sensor::BinarySensor *discharging_binary_sensor) {
    discharging_binary_sensor_ = discharging_binary_sensor;
  }
  void set_precharging_binary_sensor(binary_sensor::BinarySensor *precharging_binary_sensor) {
    precharging_binary_sensor_ = precharging_binary_sensor;
  }
  void set_heat_binary_sensor(binary_sensor::BinarySensor *heat_binary_sensor) {
    heat_binary_sensor_ = heat_binary_sensor;
  }
  void set_fan_binary_sensor(binary_sensor::BinarySensor *fan_binary_sensor) { fan_binary_sensor_ = fan_binary_sensor; }
  void set_online_status_binary_sensor(binary_sensor::BinarySensor *online_status_binary_sensor) {
    online_status_binary_sensor_ = online_status_binary_sensor;
  }

  void set_operation_status_text_sensor(text_sensor::TextSensor *operation_status_text_sensor) {
    operation_status_text_sensor_ = operation_status_text_sensor;
  }
  void set_errors_text_sensor(text_sensor::TextSensor *errors_text_sensor) { errors_text_sensor_ = errors_text_sensor; }
  void set_protect_text_sensor(text_sensor::TextSensor *protect_text_sensor) {
    protect_text_sensor_ = protect_text_sensor;
  }
  void set_device_model_text_sensor(text_sensor::TextSensor *device_model_text_sensor) {
    device_model_text_sensor_ = device_model_text_sensor;
  }

  void set_rated_capacity_sensor(sensor::Sensor *rated_capacity_sensor) {
    rated_capacity_sensor_ = rated_capacity_sensor;
  }
  void set_max_temperature_sensor(sensor::Sensor *max_temperature_sensor) {
    max_temperature_sensor_ = max_temperature_sensor;
  }
  void set_min_temperature_sensor(sensor::Sensor *min_temperature_sensor) {
    min_temperature_sensor_ = min_temperature_sensor;
  }
  void set_average_temperature_sensor(sensor::Sensor *average_temperature_sensor) {
    average_temperature_sensor_ = average_temperature_sensor;
  }
  void set_charge_voltage_limit_sensor(sensor::Sensor *charge_voltage_limit_sensor) {
    charge_voltage_limit_sensor_ = charge_voltage_limit_sensor;
  }
  void set_charge_current_limit_sensor(sensor::Sensor *charge_current_limit_sensor) {
    charge_current_limit_sensor_ = charge_current_limit_sensor;
  }
  void set_discharge_voltage_limit_sensor(sensor::Sensor *discharge_voltage_limit_sensor) {
    discharge_voltage_limit_sensor_ = discharge_voltage_limit_sensor;
  }
  void set_discharge_current_limit_sensor(sensor::Sensor *discharge_current_limit_sensor) {
    discharge_current_limit_sensor_ = discharge_current_limit_sensor;
  }
  void set_balancing_bitmask_sensor(sensor::Sensor *balancing_bitmask_sensor) {
    balancing_bitmask_sensor_ = balancing_bitmask_sensor;
  }
  void set_balancing_binary_sensor(binary_sensor::BinarySensor *balancing_binary_sensor) {
    balancing_binary_sensor_ = balancing_binary_sensor;
  }
  void set_firmware_version_text_sensor(text_sensor::TextSensor *firmware_version_text_sensor) {
    firmware_version_text_sensor_ = firmware_version_text_sensor;
  }

  void set_charging_switch(switch_::Switch *charging_switch) { charging_switch_ = charging_switch; }
  void set_discharging_switch(switch_::Switch *discharging_switch) { discharging_switch_ = discharging_switch; }

  bool change_mosfet_status(uint8_t bit, bool state);

 protected:
  void on_pack_status_(const std::vector<uint8_t> &data);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(binary_sensor::BinarySensor *sensor, bool value);
  void publish_state_(text_sensor::TextSensor *sensor, const std::string &value);
  std::string bitmask_to_string_(const char *const messages[], uint8_t messages_size, uint32_t mask);

  sensor::Sensor *total_voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *charging_power_sensor_{nullptr};
  sensor::Sensor *discharging_power_sensor_{nullptr};
  sensor::Sensor *state_of_charge_sensor_{nullptr};
  sensor::Sensor *capacity_remaining_sensor_{nullptr};
  sensor::Sensor *nominal_capacity_sensor_{nullptr};
  sensor::Sensor *rated_capacity_sensor_{nullptr};
  sensor::Sensor *mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *max_temperature_sensor_{nullptr};
  sensor::Sensor *min_temperature_sensor_{nullptr};
  sensor::Sensor *average_temperature_sensor_{nullptr};
  sensor::Sensor *state_of_health_sensor_{nullptr};
  sensor::Sensor *operation_status_bitmask_sensor_{nullptr};
  sensor::Sensor *errors_bitmask_sensor_{nullptr};
  sensor::Sensor *protect_bitmask_sensor_{nullptr};
  sensor::Sensor *charging_cycles_sensor_{nullptr};
  sensor::Sensor *min_cell_voltage_sensor_{nullptr};
  sensor::Sensor *max_cell_voltage_sensor_{nullptr};
  sensor::Sensor *min_voltage_cell_sensor_{nullptr};
  sensor::Sensor *max_voltage_cell_sensor_{nullptr};
  sensor::Sensor *delta_cell_voltage_sensor_{nullptr};
  sensor::Sensor *average_cell_voltage_sensor_{nullptr};
  sensor::Sensor *cell_count_sensor_{nullptr};
  sensor::Sensor *charge_voltage_limit_sensor_{nullptr};
  sensor::Sensor *charge_current_limit_sensor_{nullptr};
  sensor::Sensor *discharge_voltage_limit_sensor_{nullptr};
  sensor::Sensor *discharge_current_limit_sensor_{nullptr};
  sensor::Sensor *balancing_bitmask_sensor_{nullptr};
  sensor::Sensor *cell_voltage_sensors_[32]{};
  sensor::Sensor *temperature_sensors_[6]{};

  binary_sensor::BinarySensor *balancing_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *charging_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *discharging_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *precharging_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *heat_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fan_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *online_status_binary_sensor_{nullptr};

  text_sensor::TextSensor *firmware_version_text_sensor_{nullptr};
  text_sensor::TextSensor *operation_status_text_sensor_{nullptr};
  text_sensor::TextSensor *errors_text_sensor_{nullptr};
  text_sensor::TextSensor *protect_text_sensor_{nullptr};
  text_sensor::TextSensor *device_model_text_sensor_{nullptr};

  switch_::Switch *charging_switch_{nullptr};
  switch_::Switch *discharging_switch_{nullptr};

  uint16_t mosfet_status_{0xFFFF};
};

}  // namespace esphome::jbd_bms_up_pack
