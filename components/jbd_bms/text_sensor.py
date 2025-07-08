import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import CONF_JBD_BMS_ID, JBD_BMS_COMPONENT_SCHEMA

DEPENDENCIES = ["jbd_bms"]

CODEOWNERS = ["@syssi"]

CONF_ERRORS = "errors"
CONF_OPERATION_STATUS = "operation_status"
CONF_DEVICE_MODEL = "device_model"

ICON_ERRORS = "mdi:alert-circle-outline"
ICON_OPERATION_STATUS = "mdi:heart-pulse"
ICON_DEVICE_MODEL = "mdi:chip"

TEXT_SENSORS = [
    CONF_ERRORS,
    CONF_OPERATION_STATUS,
    CONF_DEVICE_MODEL,
]

CONFIG_SCHEMA = JBD_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_ERRORS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor,
            icon=ICON_ERRORS,
        ),
        cv.Optional(CONF_OPERATION_STATUS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor,
            icon=ICON_OPERATION_STATUS,
        ),
        cv.Optional(CONF_DEVICE_MODEL): text_sensor.text_sensor_schema(
            text_sensor.TextSensor,
            icon=ICON_DEVICE_MODEL,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await text_sensor.register_text_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
