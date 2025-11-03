#pragma once

#include "jbd_bms.h"

namespace esphome {
namespace jbd_bms {

class JbdBmsUP : public JbdBms {
 public:
  JbdBmsUP() : JbdBms() { is_master_ = false; }
  bool parse_jbd_bms_byte(uint8_t byte) override;
  bool change_mosfet_status(uint8_t address, uint8_t bitmask, bool state);
  void send_command(uint8_t action, uint8_t function) override;
  void send_command(uint8_t function, uint16_t start_address, uint16_t end_address, const std::vector<uint8_t> &data);
  bool write_register(uint8_t address, uint16_t value) override { return false; }
  void on_jbd_bms_data(const uint8_t &function, const uint16_t &reg_address, const std::vector<uint8_t> &data);
  void set_is_master(bool state) { is_master_ = state; }
  void add_battery(JbdBmsUP *battery) { batteries_.push_back(battery); }
  void set_battery_address(uint8_t address) { address_ = address; }
  uint8_t address() { return address_; }

  void set_mosfet_temperature_sensor(sensor::Sensor *mosfet_temperature_sensor) {
    mosfet_temperature_sensor_ = mosfet_temperature_sensor;
  }
  void set_ambient_temperature_sensor(sensor::Sensor *ambient_temperature_sensor) {
    ambient_temperature_sensor_ = ambient_temperature_sensor;
  }
  void set_state_of_health_sensor(sensor::Sensor *state_of_health_sensor) {
    state_of_health_sensor_ = state_of_health_sensor;
  }
  void set_precharging_binary_sensor(binary_sensor::BinarySensor *precharging_binary_sensor) {
    precharging_binary_sensor_ = precharging_binary_sensor;
  }
  void set_fan_binary_sensor(binary_sensor::BinarySensor *fan_binary_sensor) { fan_binary_sensor_ = fan_binary_sensor; }
  void set_heat_binary_sensor(binary_sensor::BinarySensor *heat_binary_sensor) {
    heat_binary_sensor_ = heat_binary_sensor;
  }

 protected:
  void on_pack_status_(const std::vector<uint8_t> &data);

  std::vector<JbdBmsUP *> batteries_;
  uint8_t address_{1};

  sensor::Sensor *mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *state_of_health_sensor_{nullptr};

  binary_sensor::BinarySensor *precharging_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *heat_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fan_binary_sensor_{nullptr};
};

}  // namespace jbd_bms
}  // namespace esphome
