#pragma once
#include <cstdint>
#include <vector>

namespace esphome::jbd_bms_up::testing {

#ifndef JBD_BMS_PACK_STATUS_FRAMES_DEFINED
#define JBD_BMS_PACK_STATUS_FRAMES_DEFINED

// ── Pack status response addr=1 (0x1000) ──────────────────────────────────────
//
// Real hardware capture: JBD-UP16S020, addr=1, 16S LiFePO4 100Ah 48V.
// Full frame: header[8] + payload[158] + CRC[2] = 168B total.
// Source: docs/pdus/JBD-UP16S020-two-devices.txt
//
// Decoded key values:
//   total_voltage:         53.17 V    current:               0.00 A
//   state_of_charge:       60.95 %    capacity_remaining:   60.95 Ah
//   nominal_capacity:     100.00 Ah   charging_cycles:          12
//   mosfet_temperature:    21.0 °C    ambient_temperature:   22.6 °C
//   operation_status:      0 (Idle)   state_of_health:       100 %
//   MOSFET status:         0x0003     charging: true, discharging: true
//   max_voltage_cell:      3 (1-idx)  max_cell_voltage:      3.331 V
//   min_voltage_cell:      8 (1-idx)  min_cell_voltage:      3.317 V
//   avg_cell_voltage:      3.323 V    delta_cell_voltage:    0.014 V
//   cell_count:       16         temp[0..3]: 20.9, 20.4, 21.0, 21.2 °C
//   device_model:          "JBD48100000"

// clang-format off
static const std::vector<uint8_t> PACK_STATUS_RESPONSE = {
    // header: addr=1, FC=0x78, start=0x1000, end=0x10A0, data_len=158
    0x01, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x9e,
    // data[0-7]: total_voltage=5317(53.17V), reserved, current=300000(0.00A)
    0x14, 0xc5, 0x00, 0x00, 0x00, 0x04, 0x93, 0xe0,
    // data[8-15]: soc=6095(60.95%), cap_rem=6095, nom_cap=10000(100Ah), reserved
    0x17, 0xcf, 0x17, 0xcf, 0x27, 0x10, 0x27, 0x10,
    // data[16-23]: mosfet_temp=710(21.0°C), amb_temp=726(22.6°C), op=0(Idle), soh=100
    0x02, 0xc6, 0x02, 0xd6, 0x00, 0x00, 0x00, 0x64,
    // data[24-31]: protect_bitmask=0, errors_bitmask=0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // data[32-39]: mos=0x0003(CHG+DSG), reserved, cycles=12, max_cell_idx=3
    0x00, 0x03, 0x00, 0x04, 0x00, 0x0c, 0x00, 0x03,
    // data[40-47]: max_cell_v=3331mV, min_cell_idx=8, min_cell_v=3317mV, avg=3323mV
    0x0d, 0x03, 0x00, 0x08, 0x0c, 0xf5, 0x0c, 0xfb,
    // data[48-65]: reserved (18 bytes)
    0x00, 0x04, 0x02, 0xc8, 0x00, 0x02, 0x02, 0xc0,
    0x02, 0xc4, 0x02, 0x48, 0x07, 0xd0, 0x01, 0xc0,
    // data[64-71]: reserved, num_cells=16, cell[0]=3326mV, cell[1]=3318mV
    0x07, 0xd0, 0x00, 0x10, 0x0c, 0xfe, 0x0c, 0xf6,
    // data[72-87]: cell[2]=3331V, cell[3]=3318V, cell[4]=3320V, cell[5]=3323V
    //              cell[6]=3328V, cell[7]=3317V (min), cell[8]=3324V, cell[9]=3323V
    0x0d, 0x03, 0x0c, 0xf6, 0x0c, 0xf8, 0x0c, 0xfb,
    0x0d, 0x00, 0x0c, 0xf5, 0x0c, 0xfc, 0x0c, 0xfb,
    // data[88-103]: cell[10..15], num_temps=4, temp[0]=709(20.9°C)
    0x0c, 0xfe, 0x0c, 0xfb, 0x0c, 0xfc, 0x0c, 0xfd,
    0x0c, 0xfd, 0x0c, 0xfa, 0x00, 0x04, 0x02, 0xc5,
    // data[104-115]: temp[1]=704(20.4°C), temp[2]=710(21.0°C), temp[3]=712(21.2°C), reserved
    0x02, 0xc0, 0x02, 0xc6, 0x02, 0xc8, 0x00, 0x00,
    // data[112-127]: reserved, device_model[0..7]="JBD481000"
    0x00, 0x00, 0x0c, 0x04, 0x4a, 0x42, 0x44, 0x34,
    // data[128-145]: device_model[8..] "00000" + null padding
    0x38, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // data[144-157]: model padding end + trailing reserved
    0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x5a, 0xa6,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // CRC16 (LSB-first): 0x2E6A
    0x6a, 0x2e,
};
// clang-format on

// Payload only — passed directly to on_pack_status_() in decode tests
static const std::vector<uint8_t> PACK_STATUS_FRAME(PACK_STATUS_RESPONSE.begin() + 8, PACK_STATUS_RESPONSE.end() - 2);

// ── Pack status response addr=2 (0x1000) ──────────────────────────────────────
//
// Real hardware capture: JBD-UP16S020, addr=2, same RS485 bus as addr=1.
// Full frame: header[8] + payload[150] + CRC[2] = 160B total.
// Source: docs/pdus/JBD-UP16S020-two-devices.txt
//
// Decoded key values:
//   total_voltage:         53.21 V    current:               0.00 A
//   state_of_charge:       73.00 %    capacity_remaining:   73.27 Ah
//   nominal_capacity:     100.00 Ah   charging_cycles:           2
//   mosfet_temperature:    21.4 °C    ambient_temperature:   23.0 °C
//   operation_status:      0 (Idle)   state_of_health:       100 %
//   MOSFET status:         0x0003     charging: true, discharging: true
//   max_voltage_cell:      4 (1-idx)  max_cell_voltage:      3.329 V
//   min_voltage_cell:      1 (1-idx)  min_cell_voltage:      3.322 V
//   avg_cell_voltage:      3.325 V    delta_cell_voltage:    0.007 V
//   cell_count:       16         temp[0..3]: 21.4, 21.3, 22.0, 21.8 °C
//   device_model:          "JBD48100000"

// clang-format off
static const std::vector<uint8_t> PACK_STATUS_RESPONSE_ADDR2 = {
    // header: addr=2, FC=0x78, start=0x1000, end=0x10A0, data_len=150
    0x02, 0x78, 0x10, 0x00, 0x10, 0xa0, 0x00, 0x96,
    // data[0-7]: total_voltage=5321(53.21V), reserved, current=300000(0.00A)
    0x14, 0xc9, 0x14, 0xc9, 0x00, 0x04, 0x93, 0xe0,
    // data[8-15]: soc=7300(73.00%), cap_rem=7327, nom_cap=10000(100Ah), reserved
    0x1c, 0x84, 0x1c, 0x9f, 0x27, 0x10, 0x27, 0x10,
    // data[16-23]: mosfet_temp=714(21.4°C), amb_temp=730(23.0°C), op=0(Idle), soh=100
    0x02, 0xca, 0x02, 0xda, 0x00, 0x00, 0x00, 0x64,
    // data[24-31]: protect_bitmask=0, errors_bitmask=0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // data[32-39]: mos=0x0003(CHG+DSG), reserved, cycles=2, max_cell_idx=4
    0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04,
    // data[40-47]: max_cell_v=3329mV, min_cell_idx=1, min_cell_v=3322mV, avg=3325mV
    0x0d, 0x01, 0x00, 0x01, 0x0c, 0xfa, 0x0c, 0xfd,
    // data[48-65]: reserved (18 bytes)
    0x00, 0x03, 0x02, 0xd0, 0x00, 0x02, 0x02, 0xc9,
    0x02, 0xcc, 0x02, 0x48, 0x03, 0xe8, 0x01, 0xc0,
    // data[64-71]: reserved, num_cells=16, cell[0]=3322mV, cell[1]=3327mV
    0x03, 0xe8, 0x00, 0x10, 0x0c, 0xfa, 0x0c, 0xff,
    // data[72-87]: cell[2..5], cell[6..9]
    0x0c, 0xfb, 0x0d, 0x01, 0x0c, 0xff, 0x0c, 0xfc,
    0x0c, 0xfc, 0x0d, 0x00, 0x0c, 0xfb, 0x0c, 0xfe,
    // data[88-103]: cell[10..15], num_temps=4, temp[0]=714(21.4°C)
    0x0d, 0x00, 0x0c, 0xfc, 0x0c, 0xff, 0x0c, 0xfe,
    0x0d, 0x00, 0x0c, 0xfd, 0x00, 0x04, 0x02, 0xca,
    // data[104-115]: temp[1]=713(21.3°C), temp[2]=720(22.0°C), temp[3]=718(21.8°C), reserved
    0x02, 0xc9, 0x02, 0xd0, 0x02, 0xce, 0x00, 0x00,
    // data[112-127]: reserved, device_model[0..7]="JBD481000"
    0x00, 0x00, 0x00, 0x04, 0x4a, 0x42, 0x44, 0x34,
    // data[128-149]: device_model[8..] + null padding + trailing reserved
    0x38, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x02,
    // CRC16 (LSB-first): 0xC0BC
    0xbc, 0xc0,
};
// clang-format on

static const std::vector<uint8_t> PACK_STATUS_FRAME_ADDR2(PACK_STATUS_RESPONSE_ADDR2.begin() + 8,
                                                          PACK_STATUS_RESPONSE_ADDR2.end() - 2);

// ── Charging variant ──────────────────────────────────────────────────────────
// current = +50.00A → val = 305000 = 0x0004A768
// operation_status = 1 (Charging), MOSFET = 0x0002 (CHG only)
static std::vector<uint8_t> make_pack_status_charging() {
  auto f = PACK_STATUS_FRAME;
  f[4] = 0x00;
  f[5] = 0x04;
  f[6] = 0xa7;
  f[7] = 0x68;  // current=305000
  f[20] = 0x00;
  f[21] = 0x01;  // op_status=1
  f[32] = 0x00;
  f[33] = 0x02;  // mos=CHG only
  return f;
}

// ── Error bitmask variant ─────────────────────────────────────────────────────
// errors_bitmask = 0x00010001: bit0=cell_overvoltage, bit16=eep_fault
static std::vector<uint8_t> make_pack_status_with_errors() {
  auto f = PACK_STATUS_FRAME;
  f[28] = 0x00;
  f[29] = 0x01;  // errors high word = 0x0001 (bit16 = EEP fault)
  f[30] = 0x00;
  f[31] = 0x01;  // errors low word  = 0x0001 (bit0  = cell overvoltage)
  return f;
}

#endif  // JBD_BMS_PACK_STATUS_FRAMES_DEFINED

// ── Balancing variant ─────────────────────────────────────────────────────────
// balance_status = 0x0003: bits 0+1 set → cells 1 and 2 balancing.
// Offset 112 = (68 + 16*2) + 2 + 4*2 + 2 = 116 - 4 for the 16S/4T frame.
#ifndef JBD_BMS_PACK_STATUS_BALANCING_DEFINED
#define JBD_BMS_PACK_STATUS_BALANCING_DEFINED
static std::vector<uint8_t> make_pack_status_balancing() {
  auto f = PACK_STATUS_FRAME;
  f[112] = 0x00;
  f[113] = 0x03;
  return f;
}
#endif  // JBD_BMS_PACK_STATUS_BALANCING_DEFINED

}  // namespace esphome::jbd_bms_up::testing
