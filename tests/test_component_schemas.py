"""Schema structure tests for jbd_bms ESPHome component modules."""

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import components.jbd_bms as hub  # noqa: E402
from components.jbd_bms import (  # noqa: E402  # noqa: E402
    binary_sensor,
    button,
    sensor,
    switch,
    text_sensor,
)
import components.jbd_bms_ble as hub_ble  # noqa: E402
from components.jbd_bms_ble import (  # noqa: E402
    button as ble_button,  # noqa: E402
    sensor as ble_sensor,
    switch as ble_switch,  # noqa: E402
    text_sensor as ble_text_sensor,
)


class TestHubConstants:
    def test_conf_ids_defined(self):
        assert hub.CONF_JBD_BMS_ID == "jbd_bms_id"
        assert hub_ble.CONF_JBD_BMS_BLE_ID == "jbd_bms_ble_id"


class TestSensorLists:
    def test_cells_count(self):
        assert len(sensor.CELLS) == 32
        assert len(ble_sensor.CELLS) == 32

    def test_cells_naming(self):
        assert sensor.CELLS[0] == "cell_voltage_1"
        assert sensor.CELLS[31] == "cell_voltage_32"
        for i, key in enumerate(sensor.CELLS, 1):
            assert key == f"cell_voltage_{i}"

    def test_temperatures_count(self):
        assert len(sensor.TEMPERATURES) == 6
        assert len(ble_sensor.TEMPERATURES) == 6

    def test_temperatures_naming(self):
        assert sensor.TEMPERATURES[0] == "temperature_1"
        assert sensor.TEMPERATURES[5] == "temperature_6"
        for i, key in enumerate(sensor.TEMPERATURES, 1):
            assert key == f"temperature_{i}"

    def test_sensor_defs_completeness(self):
        assert "total_voltage" in sensor.SENSOR_DEFS
        assert "state_of_charge" in sensor.SENSOR_DEFS
        assert sensor.CONF_MIN_CELL_VOLTAGE in sensor.SENSOR_DEFS
        assert sensor.CONF_MAX_CELL_VOLTAGE in sensor.SENSOR_DEFS
        assert sensor.CONF_DELTA_CELL_VOLTAGE in sensor.SENSOR_DEFS
        assert len(sensor.SENSOR_DEFS) == 32

    def test_no_cell_keys_in_sensor_defs(self):
        for key in sensor.SENSOR_DEFS:
            assert key not in sensor.CELLS
            assert key not in sensor.TEMPERATURES


class TestBinarySensorConstants:
    def test_binary_sensor_defs_dict(self):
        from components.jbd_bms.const import CONF_CHARGING, CONF_DISCHARGING

        assert CONF_CHARGING in binary_sensor.BINARY_SENSOR_DEFS
        assert CONF_DISCHARGING in binary_sensor.BINARY_SENSOR_DEFS
        assert binary_sensor.CONF_BALANCING in binary_sensor.BINARY_SENSOR_DEFS
        assert binary_sensor.CONF_ONLINE_STATUS in binary_sensor.BINARY_SENSOR_DEFS
        assert len(binary_sensor.BINARY_SENSOR_DEFS) == 4


class TestTextSensorConstants:
    def test_text_sensors_list(self):
        assert text_sensor.CONF_ERRORS in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_OPERATION_STATUS in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_DEVICE_MODEL in text_sensor.TEXT_SENSORS
        assert len(text_sensor.TEXT_SENSORS) == 3

    def test_ble_text_sensors_list(self):
        assert ble_text_sensor.CONF_ERRORS in ble_text_sensor.TEXT_SENSORS
        assert len(ble_text_sensor.TEXT_SENSORS) == 3


class TestSwitchConstants:
    def test_switches_dict(self):
        from components.jbd_bms.const import CONF_CHARGING, CONF_DISCHARGING

        assert CONF_DISCHARGING in switch.SWITCHES
        assert CONF_CHARGING in switch.SWITCHES
        assert switch.CONF_BALANCER in switch.SWITCHES
        assert len(switch.SWITCHES) == 3

    def test_ble_switches_dict(self):
        from components.jbd_bms_ble.const import CONF_CHARGING, CONF_DISCHARGING

        assert CONF_DISCHARGING in ble_switch.SWITCHES
        assert CONF_CHARGING in ble_switch.SWITCHES
        assert ble_switch.CONF_BALANCER in ble_switch.SWITCHES
        assert len(ble_switch.SWITCHES) == 3


class TestButtonConstants:
    def test_buttons_dict(self):
        assert button.CONF_RETRIEVE_HARDWARE_VERSION in button.BUTTONS
        assert button.CONF_RETRIEVE_ERROR_COUNTS in button.BUTTONS
        assert button.CONF_FORCE_SOC_RESET in button.BUTTONS
        assert len(button.BUTTONS) == 3

    def test_button_addresses_are_unique(self):
        addresses = list(button.BUTTONS.values())
        assert len(addresses) == len(set(addresses))

    def test_ble_buttons_dict(self):
        assert ble_button.CONF_RETRIEVE_HARDWARE_VERSION in ble_button.BUTTONS
        assert len(ble_button.BUTTONS) == 3
