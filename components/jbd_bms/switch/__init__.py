import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID

from .. import CONF_JBD_BMS_ID, JbdBms, jbd_bms_ns
from ..const import CONF_CHARGING, CONF_DISCHARGING

DEPENDENCIES = ["jbd_bms"]

CODEOWNERS = ["@syssi"]

CONF_BALANCER = "balancer"
CONF_BLUETOOTH = "bluetooth"
CONF_BUZZER = "buzzer"

ICON_DISCHARGING = "mdi:battery-charging-50"
ICON_CHARGING = "mdi:battery-charging-50"
ICON_BALANCER = "mdi:seesaw"
ICON_BLUETOOTH = "mdi:bluetooth"
ICON_BUZZER = "mdi:volume-high"

SWITCHES = {
    CONF_DISCHARGING: [0xE1, 1],
    CONF_CHARGING: [0xE1, 0],
    CONF_BALANCER: [0x2D, 3],
}

JbdSwitch = jbd_bms_ns.class_("JbdSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_JBD_BMS_ID): cv.use_id(JbdBms),
        cv.Optional(CONF_DISCHARGING): switch.SWITCH_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(JbdSwitch),
                cv.Optional(CONF_ICON, default=ICON_DISCHARGING): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CHARGING): switch.SWITCH_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(JbdSwitch),
                cv.Optional(CONF_ICON, default=ICON_CHARGING): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_BALANCER): switch.SWITCH_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(JbdSwitch),
                cv.Optional(CONF_ICON, default=ICON_BALANCER): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_ID])
    for key, address in SWITCHES.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await switch.register_switch(var, conf)
            cg.add(getattr(hub, f"set_{key}_switch")(var))
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address[0]))
            cg.add(var.set_bitmask(address[1]))
