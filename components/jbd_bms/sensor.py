import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_CURRENT,
    CONF_POWER,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_EMPTY,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_EMPTY,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
)

from . import CONF_JBD_BMS_ID, JBD_BMS_COMPONENT_SCHEMA

DEPENDENCIES = ["jbd_bms"]

CODEOWNERS = ["@syssi"]

CONF_STATE_OF_CHARGE = "state_of_charge"
CONF_NOMINAL_CAPACITY = "nominal_capacity"
CONF_CHARGING_CYCLES = "charging_cycles"
CONF_CAPACITY_REMAINING = "capacity_remaining"
CONF_BATTERY_CYCLE_CAPACITY = "battery_cycle_capacity"
CONF_TOTAL_VOLTAGE = "total_voltage"
CONF_CHARGING_POWER = "charging_power"
CONF_DISCHARGING_POWER = "discharging_power"

CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MIN_VOLTAGE_CELL = "min_voltage_cell"
CONF_MAX_VOLTAGE_CELL = "max_voltage_cell"
CONF_DELTA_CELL_VOLTAGE = "delta_cell_voltage"
CONF_AVERAGE_CELL_VOLTAGE = "average_cell_voltage"
CONF_OPERATION_STATUS_BITMASK = "operation_status_bitmask"
CONF_ERRORS_BITMASK = "errors_bitmask"
CONF_BALANCER_STATUS_BITMASK = "balancer_status_bitmask"
CONF_BATTERY_STRINGS = "battery_strings"
CONF_SOFTWARE_VERSION = "software_version"
CONF_SHORT_CIRCUIT_ERROR_COUNT = "short_circuit_error_count"
CONF_CHARGE_OVERCURRENT_ERROR_COUNT = "charge_overcurrent_error_count"
CONF_DISCHARGE_OVERCURRENT_ERROR_COUNT = "discharge_overcurrent_error_count"
CONF_CELL_OVERVOLTAGE_ERROR_COUNT = "cell_overvoltage_error_count"
CONF_CELL_UNDERVOLTAGE_ERROR_COUNT = "cell_undervoltage_error_count"
CONF_CHARGE_OVERTEMPERATURE_ERROR_COUNT = "charge_overtemperature_error_count"
CONF_CHARGE_UNDERTEMPERATURE_ERROR_COUNT = "charge_undertemperature_error_count"
CONF_DISCHARGE_OVERTEMPERATURE_ERROR_COUNT = "discharge_overtemperature_error_count"
CONF_DISCHARGE_UNDERTEMPERATURE_ERROR_COUNT = "discharge_undertemperature_error_count"
CONF_BATTERY_OVERVOLTAGE_ERROR_COUNT = "battery_overvoltage_error_count"
CONF_BATTERY_UNDERVOLTAGE_ERROR_COUNT = "battery_undervoltage_error_count"

ICON_CURRENT_DC = "mdi:current-dc"
ICON_BATTERY_STRINGS = "mdi:car-battery"
ICON_CAPACITY_REMAINING = "mdi:battery-50"
ICON_NOMINAL_CAPACITY = "mdi:battery-50"
ICON_CHARGING_CYCLES = "mdi:battery-sync"
ICON_BATTERY_CYCLE_CAPACITY = "mdi:battery-50"
ICON_MIN_CELL_VOLTAGE = "mdi:battery-minus-outline"
ICON_MAX_CELL_VOLTAGE = "mdi:battery-plus-outline"
ICON_MIN_VOLTAGE_CELL = "mdi:battery-minus-outline"
ICON_MAX_VOLTAGE_CELL = "mdi:battery-plus-outline"
ICON_OPERATION_STATUS_BITMASK = "mdi:heart-pulse"
ICON_ERRORS_BITMASK = "mdi:alert-circle-outline"
ICON_BALANCER_STATUS_BITMASK = "mdi:seesaw"
ICON_SOFTWARE_VERSION = "mdi:numeric"

UNIT_SECONDS = "s"
UNIT_HOURS = "h"
UNIT_AMPERE_HOURS = "Ah"

CELLS = [f"cell_voltage_{i}" for i in range(1, 33)]

TEMPERATURES = [f"temperature_{i}" for i in range(1, 7)]

# key: sensor_schema kwargs
SENSOR_DEFS = {
    CONF_STATE_OF_CHARGE: {
        "unit_of_measurement": UNIT_PERCENT,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_BATTERY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_TOTAL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CURRENT: {
        "unit_of_measurement": UNIT_AMPERE,
        "icon": ICON_CURRENT_DC,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_POWER: {
        "unit_of_measurement": UNIT_WATT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGING_POWER: {
        "unit_of_measurement": UNIT_WATT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DISCHARGING_POWER: {
        "unit_of_measurement": UNIT_WATT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_NOMINAL_CAPACITY: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_NOMINAL_CAPACITY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGING_CYCLES: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_CHARGING_CYCLES,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_CAPACITY_REMAINING: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_CAPACITY_REMAINING,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_MIN_CELL_VOLTAGE,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_MAX_CELL_VOLTAGE,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_VOLTAGE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_MIN_VOLTAGE_CELL,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_VOLTAGE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_MAX_VOLTAGE_CELL,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DELTA_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 4,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_AVERAGE_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 4,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_OPERATION_STATUS_BITMASK: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_OPERATION_STATUS_BITMASK,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_ERRORS_BITMASK: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_ERRORS_BITMASK,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_BALANCER_STATUS_BITMASK: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_BALANCER_STATUS_BITMASK,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_BATTERY_STRINGS: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_BATTERY_STRINGS,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_SOFTWARE_VERSION: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_SOFTWARE_VERSION,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_SHORT_CIRCUIT_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGE_OVERCURRENT_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DISCHARGE_OVERCURRENT_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CELL_OVERVOLTAGE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CELL_UNDERVOLTAGE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGE_OVERTEMPERATURE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGE_UNDERTEMPERATURE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DISCHARGE_OVERTEMPERATURE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DISCHARGE_UNDERTEMPERATURE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_BATTERY_OVERVOLTAGE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_BATTERY_UNDERVOLTAGE_ERROR_COUNT: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": "mdi:counter",
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_BATTERY_CYCLE_CAPACITY: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_BATTERY_CYCLE_CAPACITY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
}

_CELL_VOLTAGE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    icon=ICON_EMPTY,
    accuracy_decimals=3,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)

_TEMPERATURE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    icon=ICON_EMPTY,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CONFIG_SCHEMA = (
    JBD_BMS_COMPONENT_SCHEMA.extend(
        {
            cv.Optional(key): sensor.sensor_schema(**kwargs)
            for key, kwargs in SENSOR_DEFS.items()
        }
    )
    .extend({cv.Optional(key): _CELL_VOLTAGE_SCHEMA for key in CELLS})
    .extend({cv.Optional(key): _TEMPERATURE_SCHEMA for key in TEMPERATURES})
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_ID])
    for i, key in enumerate(TEMPERATURES):
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(hub.set_temperature_sensor(i, sens))
    for i, key in enumerate(CELLS):
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(hub.set_cell_voltage_sensor(i, sens))
    for key in SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
