import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID

from .. import CONF_JBD_BMS_ID, JBD_BMS_COMPONENT_SCHEMA, jbd_bms_ns

DEPENDENCIES = ["jbd_bms"]

CODEOWNERS = ["@syssi"]

CONF_RETRIEVE_HARDWARE_VERSION = "retrieve_hardware_version"

ICON_CONF_RETRIEVE_HARDWARE_VERSION = "mdi:numeric"

BUTTONS = {
    CONF_RETRIEVE_HARDWARE_VERSION: 0x05,
}

JbdButton = jbd_bms_ns.class_("JbdButton", button.Button, cg.Component)

CONFIG_SCHEMA = JBD_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_RETRIEVE_HARDWARE_VERSION): button.BUTTON_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(JbdButton),
                cv.Optional(
                    CONF_ICON, default=ICON_CONF_RETRIEVE_HARDWARE_VERSION
                ): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_ID])
    for key, address in BUTTONS.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await button.register_button(var, conf)
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
