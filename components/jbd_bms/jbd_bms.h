#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace jbd_bms {

class JbdBms : public uart::UARTDevice, public PollingComponent {
 public:
  void loop() override;
  void dump_config() override;
  void update() override;
  void setup() override;
  float get_setup_priority() const override;

  void set_balancing_binary_sensor(binary_sensor::BinarySensor *balancing_binary_sensor) {
    balancing_binary_sensor_ = balancing_binary_sensor;
  }
  void set_charging_binary_sensor(binary_sensor::BinarySensor *charging_binary_sensor) {
    charging_binary_sensor_ = charging_binary_sensor;
  }
  void set_discharging_binary_sensor(binary_sensor::BinarySensor *discharging_binary_sensor) {
    discharging_binary_sensor_ = discharging_binary_sensor;
  }
  void set_online_status_binary_sensor(binary_sensor::BinarySensor *online_status_binary_sensor) {
    online_status_binary_sensor_ = online_status_binary_sensor;
  }
  void set_state_of_charge_sensor(sensor::Sensor *state_of_charge_sensor) {
    state_of_charge_sensor_ = state_of_charge_sensor;
  }
  void set_total_voltage_sensor(sensor::Sensor *total_voltage_sensor) { total_voltage_sensor_ = total_voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_charging_power_sensor(sensor::Sensor *charging_power_sensor) {
    charging_power_sensor_ = charging_power_sensor;
  }
  void set_discharging_power_sensor(sensor::Sensor *discharging_power_sensor) {
    discharging_power_sensor_ = discharging_power_sensor;
  }
  void set_nominal_capacity_sensor(sensor::Sensor *nominal_capacity_sensor) {
    nominal_capacity_sensor_ = nominal_capacity_sensor;
  }
  void set_charging_cycles_sensor(sensor::Sensor *charging_cycles_sensor) {
    charging_cycles_sensor_ = charging_cycles_sensor;
  }
  void set_capacity_remaining_sensor(sensor::Sensor *capacity_remaining_sensor) {
    capacity_remaining_sensor_ = capacity_remaining_sensor;
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
  void set_operation_status_bitmask_sensor(sensor::Sensor *operation_status_bitmask_sensor) {
    operation_status_bitmask_sensor_ = operation_status_bitmask_sensor;
  }
  void set_errors_bitmask_sensor(sensor::Sensor *errors_bitmask_sensor) {
    errors_bitmask_sensor_ = errors_bitmask_sensor;
  }
  void set_balancer_status_bitmask_sensor(sensor::Sensor *balancer_status_bitmask_sensor) {
    balancer_status_bitmask_sensor_ = balancer_status_bitmask_sensor;
  }
  void set_battery_strings_sensor(sensor::Sensor *battery_strings_sensor) {
    battery_strings_sensor_ = battery_strings_sensor;
  }
  void set_temperature_sensors_sensor(sensor::Sensor *temperature_sensors_sensor) {
    temperature_sensors_sensor_ = temperature_sensors_sensor;
  }
  void set_software_version_sensor(sensor::Sensor *software_version_sensor) {
    software_version_sensor_ = software_version_sensor;
  }
  void set_cell_voltage_sensor(uint8_t cell, sensor::Sensor *cell_voltage_sensor) {
    this->cells_[cell].cell_voltage_sensor_ = cell_voltage_sensor;
  }
  void set_temperature_sensor(uint8_t temperature, sensor::Sensor *temperature_sensor) {
    this->temperatures_[temperature].temperature_sensor_ = temperature_sensor;
  }

  void set_charging_switch(switch_::Switch *charging_switch) { charging_switch_ = charging_switch; }
  void set_discharging_switch(switch_::Switch *discharging_switch) { discharging_switch_ = discharging_switch; }
  void set_balancer_switch(switch_::Switch *balancer_switch) { balancer_switch_ = balancer_switch; }

  void set_operation_status_text_sensor(text_sensor::TextSensor *operation_status_text_sensor) {
    operation_status_text_sensor_ = operation_status_text_sensor;
  }
  void set_errors_text_sensor(text_sensor::TextSensor *errors_text_sensor) { errors_text_sensor_ = errors_text_sensor; }
  void set_device_model_text_sensor(text_sensor::TextSensor *device_model_text_sensor) {
    device_model_text_sensor_ = device_model_text_sensor;
  }
  void set_enable_fake_traffic(bool enable_fake_traffic) { enable_fake_traffic_ = enable_fake_traffic; }
  void set_rx_timeout(uint16_t rx_timeout) { rx_timeout_ = rx_timeout; }
  void write_register(uint8_t address, uint16_t value);
  void change_mosfet_status(uint8_t address, uint8_t bitmask, bool state);

 protected:
  binary_sensor::BinarySensor *balancing_binary_sensor_;
  binary_sensor::BinarySensor *charging_binary_sensor_;
  binary_sensor::BinarySensor *discharging_binary_sensor_;
  binary_sensor::BinarySensor *online_status_binary_sensor_;

  sensor::Sensor *state_of_charge_sensor_;
  sensor::Sensor *total_voltage_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *power_sensor_;
  sensor::Sensor *charging_power_sensor_;
  sensor::Sensor *discharging_power_sensor_;
  sensor::Sensor *nominal_capacity_sensor_;
  sensor::Sensor *charging_cycles_sensor_;
  sensor::Sensor *capacity_remaining_sensor_;
  sensor::Sensor *min_cell_voltage_sensor_;
  sensor::Sensor *max_cell_voltage_sensor_;
  sensor::Sensor *min_voltage_cell_sensor_;
  sensor::Sensor *max_voltage_cell_sensor_;
  sensor::Sensor *delta_cell_voltage_sensor_;
  sensor::Sensor *average_cell_voltage_sensor_;
  sensor::Sensor *operation_status_bitmask_sensor_;
  sensor::Sensor *errors_bitmask_sensor_;
  sensor::Sensor *balancer_status_bitmask_sensor_;
  sensor::Sensor *battery_strings_sensor_;
  sensor::Sensor *temperature_sensors_sensor_;
  sensor::Sensor *software_version_sensor_;

  switch_::Switch *charging_switch_;
  switch_::Switch *discharging_switch_;
  switch_::Switch *balancer_switch_;

  text_sensor::TextSensor *operation_status_text_sensor_;
  text_sensor::TextSensor *errors_text_sensor_;
  text_sensor::TextSensor *device_model_text_sensor_;

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[32];

  struct Temperature {
    sensor::Sensor *temperature_sensor_{nullptr};
  } temperatures_[6];

  // @TODO:
  //
  // Cycle life
  // Production date
  // Balance status bitmask (32 Bits)
  // Protection status bitmask (16 Bits)
  // Version

  std::string device_model_{""};
  std::vector<uint8_t> rx_buffer_;
  uint32_t last_byte_{0};
  uint16_t rx_timeout_{150};
  uint8_t no_response_count_{0};
  uint8_t mosfet_status_{255};
  bool enable_fake_traffic_;

  void on_jbd_bms_data_(const uint8_t &function, const std::vector<uint8_t> &data);
  void on_cell_info_data_(const std::vector<uint8_t> &data);
  void on_hardware_info_data_(const std::vector<uint8_t> &data);
  void on_hardware_version_data_(const std::vector<uint8_t> &data);
  bool parse_jbd_bms_byte_(uint8_t byte);
  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(switch_::Switch *obj, const bool &state);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void publish_device_unavailable_();
  void reset_online_status_tracker_();
  void track_online_status_();
  void send_command_(uint8_t action, uint8_t function);
  std::string error_bits_to_string_(uint16_t bitmask);

  uint16_t chksum_(const uint8_t data[], const uint16_t len) {
    uint16_t checksum = 0x00;
    for (uint16_t i = 0; i < len; i++) {
      checksum = checksum - data[i];
    }
    return checksum;
  }
};

}  // namespace jbd_bms
}  // namespace esphome
