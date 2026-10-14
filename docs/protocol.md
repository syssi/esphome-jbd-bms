# JBD-BMS UART/BLE protocol subset

This page describes the frame format and commands currently used by the
`jbd_bms` UART and `jbd_bms_ble` components. It is a reference for this
implementation, not a complete specification for every JBD model or firmware.
The `jbd_bms_up` components use a separate protocol and are not covered here.

For model-specific details, also consult the [JBD communication protocol
PDF](Jiabaida.communication.protocol.pdf) and the repository's [UART
captures](pdus/JBD-SP04S034-L4S-200A-B-U.txt) and [BLE
captures](pdus/JBD-DP04S007L4S100A-BLE.txt).

## Frame format

A UART request has this layout:

```text
DD | function | address | length | payload... | checksum high | checksum low | 77
```

`DD` starts a frame and `77` ends it. The payload length is in bytes. The
component uses function `A5` for reads and `5A` for writes. For example, a
request for basic information is:

```text
DD A5 03 00 FF FD 77
```

A response has this layout:

```text
DD | function/address | status | length | payload... | checksum high | checksum low | 77
```

For a read response, the function/address byte identifies the requested data
(for example, `03` for basic information), followed by a status byte and the
payload length. The checksum is the 16-bit two's complement of the sum of the
bytes from the address (in a request) or status (in a response) through the
payload. It is transmitted high byte first. The frame builder and decoder use
the checksum helper in the [UART component](../components/jbd_bms/jbd_bms.h)
and [BLE component](../components/jbd_bms_ble/jbd_bms_ble.h).

The BLE component sends and receives these data frames over its GATT
characteristics. A BLE notification may contain only part of a frame; the
component assembles the bytes before decoding the response. BLE authentication
uses separate `FF AA` handshake frames, which are outside this page's scope.

## Commands used by the components

| Request | Operation | Component behavior |
| --- | --- | --- |
| `A5 03` | Read basic information | Sent at startup and during polling. After a valid response, the component requests cell information. |
| `A5 04` | Read cell information | Returns the cell-voltage data decoded by the component. |
| `A5 05` | Read hardware version | Available through the `retrieve_hardware_version` button. |
| `A5 AA` | Read error counts | Available through the `retrieve_error_counts` button. |
| `A5 <configured 8-bit address>` | Read a selected register | The register address is sent as one byte by `read_eeprom_register`. |
| `5A E1` | Write MOSFET state | Used by configured charging and discharging switches. |
| `5A 0A` | Write button action | Payloads `01 00`, `07 00`, and `04 00` are used for force-SoC-reset, automatic-balancing, and clear-alarm buttons, respectively. |

The `A5` or `5A` byte is the request function; the second byte in each table
entry is its address. The write operations change BMS state. Configure the
corresponding ESPHome controls deliberately instead of sending hand-built
frames: the `5A E1` switch value is calculated from the component's last-known
state, while `5A 0A` button payloads are fixed actions. Device behavior can
vary by model and firmware.

This table lists only operations handled by the current components. Other
commands and register meanings are omitted unless they are implemented and
verified in the repository; do not infer support from a frame captured from a
different BMS model.
