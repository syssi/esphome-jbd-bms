import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv

from .. import (
    CONF_JBD_BMS_UP_PACK_ID,
    JBD_BMS_UP_PACK_COMPONENT_SCHEMA,
    jbd_bms_up_pack_ns,
)

DEPENDENCIES = ["jbd_bms_up_pack"]
CODEOWNERS = ["@syssi"]

CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"

ICON_CHARGING = "mdi:battery-charging-50"
ICON_DISCHARGING = "mdi:battery-charging-50"

JbdBmsUpSwitch = jbd_bms_up_pack_ns.class_(
    "JbdBmsUpSwitch", switch.Switch, cg.Component
)

# bit position in the MOSFET status word (byte offset 32-33 in pack status frame)
SWITCHES = {
    CONF_CHARGING: 1,  # bit 1 = CHG
    CONF_DISCHARGING: 0,  # bit 0 = DSG
}

CONFIG_SCHEMA = JBD_BMS_UP_PACK_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_CHARGING): switch.switch_schema(
            JbdBmsUpSwitch, icon=ICON_CHARGING
        ),
        cv.Optional(CONF_DISCHARGING): switch.switch_schema(
            JbdBmsUpSwitch, icon=ICON_DISCHARGING
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_UP_PACK_ID])
    for key, bit in SWITCHES.items():
        if key in config:
            var = await switch.new_switch(config[key])
            await cg.register_component(var, config[key])
            cg.add(getattr(hub, f"set_{key}_switch")(var))
            cg.add(var.set_parent(hub))
            cg.add(var.set_bit(bit))
