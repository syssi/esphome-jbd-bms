import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv

from .. import CONF_JBD_BMS_ID, JBD_BMS_COMPONENT_SCHEMA, jbd_bms_ns

DEPENDENCIES = ["jbd_bms"]

CODEOWNERS = ["@syssi"]

CONF_RETRIEVE_HARDWARE_VERSION = "retrieve_hardware_version"
CONF_RETRIEVE_ERROR_COUNTS = "retrieve_error_counts"
CONF_FORCE_SOC_RESET = "force_soc_reset"
CONF_AUTOMATIC_BALANCING = "automatic_balancing"
CONF_CLEAR_ALARM = "clear_alarm"

JBD_CMD_READ = 0xA5
JBD_CMD_WRITE = 0x5A

BUTTONS = {
    CONF_RETRIEVE_HARDWARE_VERSION: (JBD_CMD_READ, 0x05, []),
    CONF_RETRIEVE_ERROR_COUNTS: (JBD_CMD_READ, 0xAA, []),
    CONF_FORCE_SOC_RESET: (JBD_CMD_WRITE, 0x0A, [0x01, 0x00]),
    CONF_AUTOMATIC_BALANCING: (JBD_CMD_WRITE, 0x0A, [0x07, 0x00]),
    CONF_CLEAR_ALARM: (JBD_CMD_WRITE, 0x0A, [0x04, 0x00]),
}

JbdButton = jbd_bms_ns.class_("JbdButton", button.Button, cg.Component)

CONFIG_SCHEMA = JBD_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_RETRIEVE_HARDWARE_VERSION): button.button_schema(
            JbdButton,
            icon="mdi:numeric",
        ),
        cv.Optional(CONF_RETRIEVE_ERROR_COUNTS): button.button_schema(
            JbdButton,
            icon="mdi:counter",
        ),
        cv.Optional(CONF_FORCE_SOC_RESET): button.button_schema(
            JbdButton,
            icon="mdi:battery-charging-100",
        ),
        cv.Optional(CONF_AUTOMATIC_BALANCING): button.button_schema(
            JbdButton,
            icon="mdi:scale-balance",
        ),
        cv.Optional(CONF_CLEAR_ALARM): button.button_schema(
            JbdButton,
            icon="mdi:alarm-off",
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_ID])
    for key, (action, register, data) in BUTTONS.items():
        if key in config:
            conf = config[key]
            var = await button.new_button(conf)
            await cg.register_component(var, conf)
            cg.add(var.set_parent(hub))
            cg.add(var.set_command(action))
            cg.add(var.set_address(register))
            if data:
                cg.add(var.set_payload(data[0], data[1]))
