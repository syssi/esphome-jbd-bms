import logging

import esphome.codegen as cg
from esphome.components import jbd_bms_up  # pylint: disable=no-name-in-module
import esphome.config_validation as cv
from esphome.const import CONF_ID

_LOGGER = logging.getLogger(__name__)

AUTO_LOAD = [
    "jbd_bms_up",
    "binary_sensor",
    "sensor",
    "switch",
    "text_sensor",
]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

def deprecated_renames(renames: dict[str, str]):
    def validator(config):
        config = config.copy()
        for old, new in renames.items():
            if old in config:
                _LOGGER.warning(
                    "'%s' is deprecated, use '%s' instead. Will be removed in a future release.",
                    old,
                    new,
                )
                config[new] = config.pop(old)
        return config

    return validator


CONF_JBD_BMS_UP_PACK_ID = "jbd_bms_up_pack_id"

jbd_bms_up_pack_ns = cg.esphome_ns.namespace("jbd_bms_up_pack")
JbdBmsUpPack = jbd_bms_up_pack_ns.class_(
    "JbdBmsUpPack", cg.PollingComponent, jbd_bms_up.JbdBmsUpDevice
)

JBD_BMS_UP_PACK_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_JBD_BMS_UP_PACK_ID): cv.use_id(JbdBmsUpPack),
    }
)

CONFIG_SCHEMA = cv.All(
    cv.require_esphome_version(2025, 11, 0),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JbdBmsUpPack),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(jbd_bms_up.jbd_bms_up_device_schema(1)),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await jbd_bms_up.register_jbd_bms_up_device(var, config)
