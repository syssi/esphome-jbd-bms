import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv

from .. import CONF_JBD_BMS_BLE_ID, JBD_BMS_BLE_COMPONENT_SCHEMA, jbd_bms_ble_ns

DEPENDENCIES = ["jbd_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_RETRIEVE_HARDWARE_VERSION = "retrieve_hardware_version"
CONF_RETRIEVE_ERROR_COUNTS = "retrieve_error_counts"
CONF_FORCE_SOC_RESET = "force_soc_reset"

BUTTONS = {
    CONF_RETRIEVE_HARDWARE_VERSION: 0x05,
    CONF_RETRIEVE_ERROR_COUNTS: 0xAA,
    CONF_FORCE_SOC_RESET: 0x0A,
}

JbdButton = jbd_bms_ble_ns.class_("JbdButton", button.Button, cg.Component)

CONFIG_SCHEMA = JBD_BMS_BLE_COMPONENT_SCHEMA.extend(
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
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_BLE_ID])
    for key, address in BUTTONS.items():
        if key in config:
            conf = config[key]
            var = await button.new_button(conf)
            await cg.register_component(var, conf)
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
