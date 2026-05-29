import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_JBD_BMS_UP_PACK_ID, JBD_BMS_UP_PACK_COMPONENT_SCHEMA

DEPENDENCIES = ["jbd_bms_up_pack"]

CODEOWNERS = ["@syssi"]

CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"
CONF_PRECHARGING = "precharging"
CONF_HEAT = "heat"
CONF_FAN = "fan"
CONF_ONLINE_STATUS = "online_status"

ICON_CHARGING = "mdi:battery-charging"
ICON_DISCHARGING = "mdi:power-plug"
ICON_PRECHARGING = "mdi:battery-charging-10"
ICON_HEAT = "mdi:radiator"
ICON_FAN = "mdi:fan"

BINARY_SENSOR_DEFS = {
    CONF_CHARGING: {"icon": ICON_CHARGING},
    CONF_DISCHARGING: {"icon": ICON_DISCHARGING},
    CONF_PRECHARGING: {"icon": ICON_PRECHARGING},
    CONF_HEAT: {"icon": ICON_HEAT},
    CONF_FAN: {"icon": ICON_FAN},
    CONF_ONLINE_STATUS: {
        "device_class": DEVICE_CLASS_CONNECTIVITY,
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
}

CONFIG_SCHEMA = JBD_BMS_UP_PACK_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(key): binary_sensor.binary_sensor_schema(**kwargs)
        for key, kwargs in BINARY_SENSOR_DEFS.items()
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_UP_PACK_ID])
    for key in BINARY_SENSOR_DEFS:
        if key in config:
            sens = await binary_sensor.new_binary_sensor(config[key])
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
