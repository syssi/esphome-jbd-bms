import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_JBD_BMS_UP_PACK_ID, JBD_BMS_UP_PACK_COMPONENT_SCHEMA

DEPENDENCIES = ["jbd_bms_up_pack"]

CODEOWNERS = ["@syssi"]

CONF_OPERATION_STATUS = "operation_status"
CONF_ERRORS = "errors"
CONF_PROTECT = "protect"
CONF_DEVICE_MODEL = "device_model"

TEXT_SENSOR_DEFS = {
    CONF_OPERATION_STATUS: {"icon": "mdi:heart-pulse"},
    CONF_ERRORS: {
        "icon": "mdi:alert-circle-outline",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_PROTECT: {
        "icon": "mdi:shield-alert-outline",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    CONF_DEVICE_MODEL: {
        "icon": "mdi:chip",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
}

CONFIG_SCHEMA = JBD_BMS_UP_PACK_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(key): text_sensor.text_sensor_schema(**kwargs)
        for key, kwargs in TEXT_SENSOR_DEFS.items()
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_UP_PACK_ID])
    for key in TEXT_SENSOR_DEFS:
        if key in config:
            sens = await text_sensor.new_text_sensor(config[key])
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
