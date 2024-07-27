import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_JBD_BMS_BLE_ID, JbdBmsBle
from .const import CONF_CHARGING, CONF_DISCHARGING

DEPENDENCIES = ["jbd_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_BALANCING = "balancing"
CONF_ONLINE_STATUS = "online_status"

ICON_CHARGING = "mdi:battery-charging"
ICON_DISCHARGING = "mdi:power-plug"
ICON_BALANCING = "mdi:battery-heart-variant"

BINARY_SENSORS = [
    CONF_CHARGING,
    CONF_DISCHARGING,
    CONF_BALANCING,
    CONF_ONLINE_STATUS,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_JBD_BMS_BLE_ID): cv.use_id(JbdBmsBle),
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
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_BLE_ID])
    for key in BINARY_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await binary_sensor.register_binary_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
