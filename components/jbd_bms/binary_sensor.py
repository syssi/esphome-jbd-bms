import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_JBD_BMS_ID, JBD_BMS_COMPONENT_SCHEMA
from .const import CONF_CHARGING, CONF_DISCHARGING

DEPENDENCIES = ["jbd_bms"]

CODEOWNERS = ["@syssi"]

CONF_BALANCING = "balancing"
CONF_ONLINE_STATUS = "online_status"
CONF_PRECHARGING = "precharging"
CONF_FAN = "fan"
CONF_HEAT = "heat"

ICON_CHARGING = "mdi:battery-charging"
ICON_DISCHARGING = "mdi:power-plug"
ICON_BALANCING = "mdi:battery-heart-variant"
ICON_FAN = "mdi:fan"
ICON_HEAT = "mdi:radiator"

BINARY_SENSORS = [
    CONF_CHARGING,
    CONF_DISCHARGING,
    CONF_BALANCING,
    CONF_ONLINE_STATUS,
    # UP ONLY
    CONF_PRECHARGING,
    CONF_HEAT,
    CONF_FAN,
]

CONFIG_SCHEMA = JBD_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_CHARGING): binary_sensor.binary_sensor_schema(
            icon=ICON_CHARGING
        ),
        cv.Optional(CONF_DISCHARGING): binary_sensor.binary_sensor_schema(
            icon=ICON_DISCHARGING
        ),
        cv.Optional(CONF_BALANCING): binary_sensor.binary_sensor_schema(
            icon=ICON_BALANCING
        ),
        cv.Optional(CONF_ONLINE_STATUS): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_PRECHARGING): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_HEAT): binary_sensor.binary_sensor_schema(icon=ICON_HEAT),
        cv.Optional(CONF_FAN): binary_sensor.binary_sensor_schema(icon=ICON_FAN),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_ID])
    for key in BINARY_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await binary_sensor.register_binary_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
