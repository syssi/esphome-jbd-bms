#pragma once
#include <cstdint>
#include <vector>

namespace esphome::jbd_bms::testing {

// Source: esp32-example-faker.yaml / docs/pdus/JBD-SP04S034-L4S-200A-B-U.txt
// 4S, 5 Ah. Decoded key values:
//   total_voltage: 15.60 V         current: 0.00 A         power: 0.0 W
//   capacity_remaining: 4.98 Ah    nominal_capacity: 5.00 Ah
//   charging_cycles: 0             soc: 100 %
//   balancer_status_bitmask: 0     errors_bitmask: 0
//   software_version: 8.0          operation_status: 0x03 → "Charging, Discharging"
//   battery_strings: 4             temperature_sensors: 3
//   temperatures[0]: 22.4°C  [1]: 22.3°C  [2]: 21.7°C
static const std::vector<uint8_t> HWINFO_FRAME = {
    0x06, 0x18, 0x00, 0x00, 0x01, 0xF2, 0x01, 0xF4, 0x00, 0x00, 0x2C, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0x64, 0x03, 0x04, 0x03, 0x0B, 0x8B, 0x0B, 0x8A, 0x0B, 0x84,
};

// Source: esp32-example-faker.yaml / docs/pdus/JBD-SP04S034-L4S-200A-B-U.txt
// 4S cell voltages. Decoded key values:
//   cell[0]: 3.909 V (max)  cell[1]: 3.901 V  cell[2]: 3.895 V (min)  cell[3]: 3.901 V
//   min_cell: 3 (1-based)   max_cell: 1 (1-based)
//   delta: 0.014 V          avg: 3.9015 V
static const std::vector<uint8_t> CELLINFO_FRAME = {
    0x0F, 0x45, 0x0F, 0x3D, 0x0F, 0x37, 0x0F, 0x3D,
};

// Source: esp32-example-faker.yaml / docs/pdus/JBD-SP04S034-L4S-200A-B-U.txt
// Device model string: "JBD-SP04S034-L4S-200A-B-U"
static const std::vector<uint8_t> HWVER_FRAME = {
    0x4A, 0x42, 0x44, 0x2D, 0x53, 0x50, 0x30, 0x34, 0x53, 0x30, 0x33, 0x34, 0x2D,
    0x4C, 0x34, 0x53, 0x2D, 0x32, 0x30, 0x30, 0x41, 0x2D, 0x42, 0x2D, 0x55,
};

// Source: real hardware capture (jbd_bms_ble BLE notification, function 0xAA).
// 24 bytes = 11 × uint16_t counters + 2 bytes unknown/unused.
// Decoded key values:
//   short_circuit: 0          charge_overcurrent: 0      discharge_overcurrent: 0
//   cell_overvoltage: 122      cell_undervoltage: 2       charge_overtemperature: 0
//   charge_undertemperature: 0 discharge_overtemperature: 0  discharge_undertemperature: 0
//   battery_overvoltage: 0     battery_undervoltage: 0
static const std::vector<uint8_t> ERRCOUNT_FRAME = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7A, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

// Source: real hardware capture (BLE log, jbd_bms_ble component).
// 4S, 280 Ah, cell overvoltage protection active. Decoded key values:
//   total_voltage: 14.28 V         current: 0.00 A
//   capacity_remaining: 280.00 Ah  nominal_capacity: 280.00 Ah
//   charging_cycles: 8             soc: 100 %
//   balancer_status_bitmask: 0     errors_bitmask: 0x0001 → cell_overvoltage_protection
//   software_version: 8.0          operation_status: 0x02 → "Discharging"
//   battery_strings: 4             temperature_sensors: 3
//   temperatures[0]: 21.5°C  [1]: 20.9°C  [2]: 20.4°C
static const std::vector<uint8_t> HWINFO_FRAME_CELL_OVERVOLTAGE = {
    0x05, 0x94, 0x00, 0x00, 0x6D, 0x60, 0x6D, 0x60, 0x00, 0x08, 0x2C, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x80, 0x64, 0x02, 0x04, 0x03, 0x0B, 0x82, 0x0B, 0x7C, 0x0B, 0x77,
};

// errors_bitmask = 0x1FFF → all 13 protection bits set
// data[16]=0x1F, data[17]=0xFF
static const std::vector<uint8_t> HWINFO_FRAME_ALL_PROTECTIONS = {
    0x06, 0x18, 0x00, 0x00, 0x01, 0xF2, 0x01, 0xF4, 0x00, 0x00, 0x2C, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x1F, 0xFF, 0x80, 0x64, 0x03, 0x04, 0x03, 0x0B, 0x8B, 0x0B, 0x8A, 0x0B, 0x84,
};

// Source: real hardware capture (README.md example output).
// 4S, 5 Ah, software MOS lock active. Decoded key values:
//   total_voltage: 15.47 V         current: 0.00 A
//   capacity_remaining: 4.93 Ah    nominal_capacity: 5.00 Ah
//   charging_cycles: 0             soc: 99 %
//   balancer_status_bitmask: 0     errors_bitmask: 0x1000 → mosfet_software_lock
//   software_version: 8.0          operation_status: 0x02 → "Discharging"
//   battery_strings: 4             temperature_sensors: 3
//   temperatures[0]: 24.5°C  [1]: 24.2°C  [2]: 23.7°C
static const std::vector<uint8_t> HWINFO_FRAME_MOSFET_SOFTWARE_LOCK = {
    0x06, 0x0B, 0x00, 0x00, 0x01, 0xED, 0x01, 0xF4, 0x00, 0x00, 0x2C, 0x7C, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x00, 0x80, 0x63, 0x02, 0x04, 0x03, 0x0B, 0xA0, 0x0B, 0x9D, 0x0B, 0x98,
};

// Synthetic frame: cells 1 and 3 balancing (4S pack). Decoded key values:
//   battery_strings: 4
//   balancer_status_bitmask: 0x0000000A → bit3=cell1, bit1=cell3
//   errors_bitmask: 0
static const std::vector<uint8_t> HWINFO_FRAME_BALANCING_CELLS_1_3 = {
    0x06, 0x18, 0x00, 0x00, 0x01, 0xF2, 0x01, 0xF4, 0x00, 0x00, 0x2C, 0x7C, 0x00, 0x00, 0x00,
    0x0A, 0x00, 0x00, 0x80, 0x64, 0x03, 0x04, 0x03, 0x0B, 0x8B, 0x0B, 0x8A, 0x0B, 0x84,
};

}  // namespace esphome::jbd_bms::testing
