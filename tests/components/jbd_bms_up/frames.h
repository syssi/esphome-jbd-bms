#pragma once
#include <cstdint>
#include <vector>

namespace esphome::jbd_bms_up::testing {

#ifndef JBD_BMS_PACK_STATUS_FRAMES_DEFINED
#define JBD_BMS_PACK_STATUS_FRAMES_DEFINED

// ── Pack status response (0x1000) ─────────────────────────────────────────────
//
// Real hardware capture: UP16S020, addr=1, 16S LiFePO4 100Ah 48V rack battery.
// Full frame: header[8] + payload[158] + CRC[2] = 168B total.
// CRC16 (Modbus, LSB-first): 0x54E2 → bytes 0xE2, 0x54.
//
// Decoded key values:
//   total_voltage:         52.90 V    current:               0.00 A
//   state_of_charge:       45.19 %    capacity_remaining:   45.19 Ah
//   nominal_capacity:     100.00 Ah   charging_cycles:          12
//   mosfet_temperature:    25.6 °C    ambient_temperature:   27.4 °C
//   operation_status:      0 (Idle)   state_of_health:       100 %
//   protect_bitmask:       0          errors_bitmask:            0
//   MOSFET status:         0x0003     charging: true, discharging: true
//   max_voltage_cell:      1          max_cell_voltage:      3.307 V
//   min_voltage_cell:      2          min_cell_voltage:      3.306 V
//   avg_cell_voltage:      3.306 V    delta_cell_voltage:    0.001 V
//   battery_strings:       16
//   cell[0]:  3.307 V  cell[3]:  3.307 V  cell[9]:  3.307 V  cell[15]: 3.307 V
//   temp[0..3]: 26.0, 25.4, 26.2, 26.4 °C
//   device_model: "JBD48100000" (null-padded to 30 bytes)
//
// Payload offset map:
//   [0-1]   total_voltage ×0.01 V         [2-3]   reserved
//   [4-7]   current 32-bit (val-300000)×0.01 A
//   [8-9]   state_of_charge ×0.01 %       [10-11] capacity_remaining ×0.01 Ah
//   [12-13] nominal_capacity ×0.01 Ah     [14-15] reserved
//   [16-17] mosfet_temperature (val-500)×0.1 °C
//   [18-19] ambient_temperature (val-500)×0.1 °C
//   [20-21] operation_status              [22-23] state_of_health %
//   [24-27] protect_bitmask 32-bit        [28-31] errors_bitmask 32-bit
//   [32-33] MOSFET status (bit0=DSG, bit1=CHG, bit2=precharge)
//   [34-35] reserved                      [36-37] charging_cycles
//   [38-39] max_voltage_cell index        [40-41] max_cell_voltage ×0.001 V
//   [42-43] min_voltage_cell index        [44-45] min_cell_voltage ×0.001 V
//   [46-47] avg_cell_voltage ×0.001 V     [48-65] reserved (18 bytes)
//   [66-67] num_cells                     [68+]   cell voltages (×0.001 V each)
//   [68+2n] num_temperature_sensors       [70+2n] temperatures (val-500)×0.1 °C
//   [+6]    reserved                      [+30]   device_model string

// clang-format off
static const std::vector<uint8_t> PACK_STATUS_RESPONSE = {
    // header: addr=1, FC=0x78, start=0x1000, end=0x10A0, data_len=158
    0x01, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x9e,
    // data[0-7]: total_voltage=5290(52.90V), reserved, current=300000(0.00A)
    0x14, 0xaa, 0x00, 0x00, 0x00, 0x04, 0x93, 0xe0,
    // data[8-15]: soc=4519(45.19%), cap_rem=4519, nom_cap=10000(100Ah), reserved
    0x11, 0xa7, 0x11, 0xa7, 0x27, 0x10, 0x27, 0x10,
    // data[16-23]: mosfet_temp=756(25.6°C), amb_temp=774(27.4°C), op=0(Idle), soh=100
    0x02, 0xf4, 0x03, 0x06, 0x00, 0x00, 0x00, 0x64,
    // data[24-31]: protect_bitmask=0, errors_bitmask=0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // data[32-37]: mos=0x0003(CHG+DSG), reserved, cycles=12
    0x00, 0x03, 0x00, 0x04, 0x00, 0x0c,
    // data[38-39]: max_voltage_cell index=1
    0x00, 0x01,
    // data[40-41]: max_cell_voltage=3307mV(3.307V)
    0x0c, 0xeb,
    // data[42-43]: min_voltage_cell index=2
    0x00, 0x02,
    // data[44-45]: min_cell_voltage=3306mV(3.306V)
    0x0c, 0xea,
    // data[46-47]: avg_cell_voltage=3306mV(3.306V)
    0x0c, 0xea,
    // data[48-65]: reserved (18 bytes)
    0x00, 0x04, 0x02, 0xfc, 0x00, 0x02, 0x02, 0xf2,
    0x02, 0xf8, 0x02, 0x48, 0x07, 0xd0, 0x01, 0xc0,
    0x07, 0xd0,
    // data[66-67]: num_cells=16
    0x00, 0x10,
    // data[68-99]: 16 cell voltages (mV, 0-indexed)
    0x0c, 0xeb,  // cell[0]  = 3307 mV = 3.307 V
    0x0c, 0xea,  // cell[1]  = 3306 mV = 3.306 V
    0x0c, 0xea,  // cell[2]  = 3306 mV
    0x0c, 0xeb,  // cell[3]  = 3307 mV
    0x0c, 0xea,  // cell[4]  = 3306 mV
    0x0c, 0xea,  // cell[5]  = 3306 mV
    0x0c, 0xea,  // cell[6]  = 3306 mV
    0x0c, 0xea,  // cell[7]  = 3306 mV
    0x0c, 0xea,  // cell[8]  = 3306 mV
    0x0c, 0xeb,  // cell[9]  = 3307 mV
    0x0c, 0xeb,  // cell[10] = 3307 mV
    0x0c, 0xeb,  // cell[11] = 3307 mV
    0x0c, 0xea,  // cell[12] = 3306 mV
    0x0c, 0xea,  // cell[13] = 3306 mV
    0x0c, 0xea,  // cell[14] = 3306 mV
    0x0c, 0xeb,  // cell[15] = 3307 mV
    // data[100-101]: num_temperature_sensors=4
    0x00, 0x04,
    // data[102-109]: 4 temperatures (26.0, 25.4, 26.2, 26.4 °C)
    0x02, 0xf8,  // temp[0] = 760 → (760-500)×0.1 = 26.0 °C
    0x02, 0xf2,  // temp[1] = 754 → 25.4 °C
    0x02, 0xfa,  // temp[2] = 762 → 26.2 °C
    0x02, 0xfc,  // temp[3] = 764 → 26.4 °C
    // data[110-115]: reserved (6 bytes, skipped after temperatures)
    0x00, 0x00, 0x00, 0x00, 0x0c, 0x04,
    // data[116-145]: device_model "JBD48100000" + null padding (30 bytes)
    0x4a, 0x42, 0x44, 0x34, 0x38, 0x31, 0x30, 0x30,  // "JBD48100"
    0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,  // "000" + padding
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // data[146-157]: trailing reserved bytes (12 bytes)
    0x00, 0x02, 0x00, 0x03, 0x5a, 0xa6, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    // CRC16 (LSB-first): 0x54E2
    0xe2, 0x54,
};
// clang-format on

// Payload only — passed directly to on_pack_status_() in decode tests
static const std::vector<uint8_t> PACK_STATUS_FRAME(PACK_STATUS_RESPONSE.begin() + 8, PACK_STATUS_RESPONSE.end() - 2);

#endif  // JBD_BMS_PACK_STATUS_FRAMES_DEFINED

}  // namespace esphome::jbd_bms_up::testing
