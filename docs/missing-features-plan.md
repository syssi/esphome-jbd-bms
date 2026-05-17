# Missing Features: ESPHome vs Android App

Analysis based on:
- GitHub issue [#196](https://github.com/syssi/esphome-jbd-bms/issues/196)
- Decompiled Android app v3.2.035 (`BMSBaseInfoCMDEntity.java`)
- Current ESPHome implementation (`jbd_bms.cpp`, `jbd_bms.h`)

---

## Register 0x03 Frame Layout (Reference)

```
Byte  Len  Content
 0     2   Total voltage       ÷100    V
 2     2   Current             ÷100    A  (signed)
 4     2   Remaining capacity  ÷100    Ah
 6     2   Nominal capacity    ÷100    Ah
 8     2   Cycle count
10     2   Production date     (packed: year-9 | month-4 | day-5)
12     4   Balance status      (bitmask, 1 bit per cell)
16     2   Protection status   (bitmask, see table below)
18     1   Software version    (hi-nibble.lo-nibble)
19     1   State of charge     %
20     1   FET state           (bit 0 = charge, bit 1 = discharge)
21     1   Cell count
22     1   NTC sensor count
23     n*2 NTC temperatures    (value - 2731) ÷ 10    °C
```

### Extended frame (newer firmware only)

Present when `frame_len > (ntcNum * 2) + 22 + 5`:

```
Offset       Len  Content
ntcNum*2+23   1   Mode flag       0x88 = high-resolution mode
ntcNum*2+24   2   Alarm bitmask   (meaning TBD)
ntcNum*2+26   2   Learned capacity ÷100   Ah
ntcNum*2+28   2   Balance current  ÷100   A
```

**High-resolution mode:** When the mode flag byte equals `0x88` (136), current and
remaining capacity use `÷10` instead of `÷100`.

---

## Protection Status Bitmask (Bytes 16–17)

The app's `formatProtectionState()` iterates 13 bits across both bytes.
Bit order: `byte[1]` holds bits 0–7, `byte[0]` holds bits 8–12.

| Bit | Byte  | Mask | Name |
|-----|-------|------|------|
|  0  | [1]   | 0x0001 | Cell overvoltage |
|  1  | [1]   | 0x0002 | Cell undervoltage |
|  2  | [1]   | 0x0004 | Pack overvoltage |
|  3  | [1]   | 0x0008 | Pack undervoltage |
|  4  | [1]   | 0x0010 | Charge overtemperature |
|  5  | [1]   | 0x0020 | Charge undertemperature |
|  6  | [1]   | 0x0040 | Discharge overtemperature |
|  7  | [1]   | 0x0080 | Discharge undertemperature |
|  8  | [0]   | 0x0100 | Charge overcurrent |
|  9  | [0]   | 0x0200 | Discharge overcurrent |
| 10  | [0]   | 0x0400 | Short circuit |
| 11  | [0]   | 0x0800 | IC front-end error |
| 12  | [0]   | 0x1000 | Software MOS lock |
| 13  | [0]   | 0x2000 | Charge timeout (ESPHome only, not in app) |

ESPHome currently reads the raw 16-bit value into `errors_bitmask_sensor_` and
formats all set bits into `errors_text_sensor_`. Individual binary sensors per
protection bit are absent.

---

## Balance Status Bitmask (Bytes 12–15)

The app's `formatBalanceStates()` maps 32 bits to a per-cell boolean array using
this byte-to-cell mapping:

| Cells   | Source byte |
|---------|-------------|
|  0–7    | byte[1] |
|  8–15   | byte[0] |
| 16–23   | byte[3] |
| 24–31   | byte[2] |

ESPHome reads the raw 32-bit value into `balancer_status_bitmask_sensor_` and
derives one global `balancing_binary_sensor_`. Individual per-cell balance
binary sensors are absent.

---

## Missing Features

### 1. Protection Status Binary Sensors — Issue #196

**Priority: High**

The app exposes each protection bit individually in the main dashboard UI
(`tvNowprotectionname`, `mProtectStatus`). ESPHome only provides the raw
bitmask and a combined text sensor.

**What to add** (both `jbd_bms` and `jbd_bms_ble` components):
- 13 optional binary sensors, one per protection bit (see table above)
- Names: `cell_overvoltage_protection`, `cell_undervoltage_protection`,
  `pack_overvoltage_protection`, `pack_undervoltage_protection`,
  `charge_overtemperature_protection`, `charge_undertemperature_protection`,
  `discharge_overtemperature_protection`, `discharge_undertemperature_protection`,
  `charge_overcurrent_protection`, `discharge_overcurrent_protection`,
  `short_circuit_protection`, `ic_frontend_error`, `software_lock_mos`
- Source: `errors_bitmask` bits 0–12, already parsed in `on_hardware_info_data_`
- Suggested `entity_category`: `diagnostic`

**Implementation notes:**
- Data already read at line 321 (`uint16_t errors_bitmask`), no new BMS commands needed
- Add sensors to `jbd_bms.h` and `jbd_bms.cpp` alongside the existing
  `errors_bitmask_sensor_` and `errors_text_sensor_`
- Same change applies identically to `jbd_bms_ble`

---

### 2. Production Date Text Sensor

**Priority: Medium**

**App behavior:** `formatProductDate(int i)` → `((i >> 9) + 2000) + "-" + ((i >> 5) & 15) + "-" + (i & 31)`

**Current ESPHome behavior:** Bytes 10–11 are decoded at line 310–312 of
`jbd_bms.cpp` and logged via `ESP_LOGD` only. No sensor is published.

**What to add:**
- `production_date` text sensor (entity_category: diagnostic)
- Format: `"YYYY-MM-DD"` (zero-padded month/day for consistency)
- Source: `jbd_get_16bit(10)`, already read

---

### 3. Extended Frame Fields (Newer Firmware)

**Priority: Medium**

Some BMS units return an extended 0x03 frame with additional bytes after the
NTC temperatures. The app detects this via `content.length > (ntcNum * 2) + 22 + 5`.

**What to add:**

| Sensor | Type | Unit | Notes |
|--------|------|------|-------|
| `learned_capacity` | sensor | Ah | Measured/calibrated capacity |
| `balance_current` | sensor | A | Current through balance resistors |
| `alarm_bitmask` | sensor | — | Raw alarm bitmask, meaning TBD |

**High-resolution current mode** (same frame):
- When mode flag byte == `0x88`, the BMS reports current with `÷10` resolution
  instead of `÷100`
- ESPHome currently always uses `÷100` → values would be 10× too small on
  affected devices
- Fix: check `content[ntcNum * 2 + 23] == 0x88` and adjust divisors accordingly

---

### 4. Per-Cell Balance State Binary Sensors

**Priority: Low** (raw bitmask already available via `balancer_status_bitmask`)

**App behavior:** `balanceStates[N]` — one boolean per cell, shown per-cell in
the voltage table.

**What to add:**
- Up to 32 optional binary sensors `balance_active_1` … `balance_active_32`,
  structured the same as `cell_voltage_1` … `cell_voltage_32`
- Derived from existing `balance_status_bitmask` (32-bit), using the byte-to-cell
  mapping described above

---

## Summary Table

| # | Feature | Priority | New sensors | Data source | Effort |
|---|---------|----------|-------------|-------------|--------|
| 1 | Protection status binary sensors | **High** | 13 binary sensors | Byte 16–17, already read | Low — no new parsing |
| 2 | Production date text sensor | Medium | 1 text sensor | Byte 10–11, already read | Low — no new parsing |
| 3 | Extended frame fields | Medium | 2–3 sensors | Bytes after NTC data | Medium — new parsing + detection |
| 4 | High-resolution current mode | Medium | — (corrects existing sensors) | Mode flag byte | Low — conditional divisor |
| 5 | Per-cell balance binary sensors | Low | Up to 32 binary sensors | Bytes 12–15, already read | Medium — new sensor array |
