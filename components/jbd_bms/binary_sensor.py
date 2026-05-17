import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_PROBLEM,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

from . import CONF_JBD_BMS_ID, JBD_BMS_COMPONENT_SCHEMA
from .const import CONF_CHARGING, CONF_DISCHARGING

DEPENDENCIES = ["jbd_bms"]

CODEOWNERS = ["@syssi"]

CONF_BALANCING = "balancing"
CONF_ONLINE_STATUS = "online_status"
CONF_CELL_OVERVOLTAGE_PROTECTION = "cell_overvoltage_protection"
CONF_CELL_UNDERVOLTAGE_PROTECTION = "cell_undervoltage_protection"
CONF_PACK_OVERVOLTAGE_PROTECTION = "pack_overvoltage_protection"
CONF_PACK_UNDERVOLTAGE_PROTECTION = "pack_undervoltage_protection"
CONF_CHARGE_OVERTEMPERATURE_PROTECTION = "charge_overtemperature_protection"
CONF_CHARGE_UNDERTEMPERATURE_PROTECTION = "charge_undertemperature_protection"
CONF_DISCHARGE_OVERTEMPERATURE_PROTECTION = "discharge_overtemperature_protection"
CONF_DISCHARGE_UNDERTEMPERATURE_PROTECTION = "discharge_undertemperature_protection"
CONF_CHARGE_OVERCURRENT_PROTECTION = "charge_overcurrent_protection"
CONF_DISCHARGE_OVERCURRENT_PROTECTION = "discharge_overcurrent_protection"
CONF_SHORT_CIRCUIT_PROTECTION = "short_circuit_protection"
CONF_IC_FRONTEND_ERROR = "ic_frontend_error"
CONF_MOSFET_SOFTWARE_LOCK = "mosfet_software_lock"

ICON_CHARGING = "mdi:battery-charging"
ICON_DISCHARGING = "mdi:power-plug"
ICON_BALANCING = "mdi:battery-heart-variant"

# key: binary_sensor_schema kwargs
BINARY_SENSOR_DEFS = {
    CONF_CHARGING: {
        "icon": ICON_CHARGING,
    },
    CONF_DISCHARGING: {
        "icon": ICON_DISCHARGING,
    },
    CONF_BALANCING: {
        "icon": ICON_BALANCING,
    },
    CONF_ONLINE_STATUS: {
        "device_class": DEVICE_CLASS_CONNECTIVITY,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_CELL_OVERVOLTAGE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_CELL_UNDERVOLTAGE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_PACK_OVERVOLTAGE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_PACK_UNDERVOLTAGE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_CHARGE_OVERTEMPERATURE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_CHARGE_UNDERTEMPERATURE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_DISCHARGE_OVERTEMPERATURE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_DISCHARGE_UNDERTEMPERATURE_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_CHARGE_OVERCURRENT_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_DISCHARGE_OVERCURRENT_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_SHORT_CIRCUIT_PROTECTION: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_IC_FRONTEND_ERROR: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_MOSFET_SOFTWARE_LOCK: {
        "device_class": DEVICE_CLASS_PROBLEM,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
}

CONFIG_SCHEMA = JBD_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(key): binary_sensor.binary_sensor_schema(**kwargs)
        for key, kwargs in BINARY_SENSOR_DEFS.items()
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_ID])
    for key in BINARY_SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
