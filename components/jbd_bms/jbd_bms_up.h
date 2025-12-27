#pragma once

#include "esphome/core/ring_buffer.h"
#include "jbd_bms.h"

namespace esphome {
namespace jbd_bms {

class JbdBmsUP : public JbdBms {
 public:
  void setup() override;
  bool parse_jbd_bms_byte(uint8_t byte) override;
  //bool change_mosfet_status(uint8_t address, uint8_t bitmask, bool state);
  void send_command(uint8_t action, uint8_t function) override;
  void send_command(uint8_t function, uint16_t start_address, uint16_t end_address, const std::vector<uint8_t> &data);
  bool write_register(uint8_t address, uint16_t value) override;
  void on_jbd_bms_data(const uint8_t &function, const uint16_t &reg_address, const std::vector<uint8_t> &data);
  void set_master(JbdBmsUP *battery) { master_ = battery; }
  void set_next_battery(JbdBmsUP *battery) { next_battery_ = battery; }
  void set_battery_address(uint8_t address) { address_ = address; }
  bool is_master() override { return master_ == this; }
  uint8_t address() { return address_; }
  JbdBmsUP * next_battery() { return this->next_battery_; }
  void set_tx_pin(InternalGPIOPin *pin) { tx_pin_ = pin; }
  void set_rx_pin(InternalGPIOPin *pin) { rx_pin_ = pin; }
  InternalGPIOPin *tx_pin() {return tx_pin_; }
  InternalGPIOPin *rx_pin() {return rx_pin_; }
  void tx_thread();
  void update_tx_wait();



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
  void set_protect_bitmask_sensor(sensor::Sensor *errors_bitmask_sensor) {
    errors_bitmask_sensor_ = errors_bitmask_sensor;
  }
  std::shared_ptr<RingBuffer> tx_ringbuffer;

 protected:
  void on_pack_status_(const std::vector<uint8_t> &data);

  JbdBmsUP * master_{nullptr};
  JbdBmsUP * next_battery_{nullptr};
  uint8_t address_{1};
  Mutex *mutex_{nullptr};
  uint32_t tx_wait_{0};
  size_t tx_ringbuffer_size_{1024};

  uint8_t current_uart_{0};
  InternalGPIOPin *tx_pin_;
  InternalGPIOPin *rx_pin_;
  void switch_uart_(uint8_t battery);

  sensor::Sensor *mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *state_of_health_sensor_{nullptr};
  sensor::Sensor *protect_bitmask_sensor_{nullptr};

  binary_sensor::BinarySensor *precharging_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *heat_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fan_binary_sensor_{nullptr};

  text_sensor::TextSensor *protect_text_sensor_{nullptr};
};

}  // namespace jbd_bms
}  // namespace esphome
