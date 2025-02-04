import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import CONF_ICON, CONF_ID

from .. import CONF_JBD_BMS_BLE_ID, JBD_BMS_BLE_COMPONENT_SCHEMA, jbd_bms_ble_ns

DEPENDENCIES = ["jbd_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_READ_EEPROM_REGISTER = "read_eeprom_register"
CONF_OPTIONSMAP = "optionsmap"

JbdSelect = jbd_bms_ble_ns.class_("JbdSelect", select.Select, cg.Component)


def ensure_option_map(value):
    cv.check_not_templatable(value)
    option = cv.All(cv.int_range(0, 2**16 - 1))
    mapping = cv.All(cv.string_strict)
    options_map_schema = cv.Schema({option: mapping})
    value = options_map_schema(value)

    all_values = list(value.keys())
    unique_values = set(value.keys())
    if len(all_values) != len(unique_values):
        raise cv.Invalid("Mapping values must be unique.")

    return value


SELECTS = {
    CONF_READ_EEPROM_REGISTER,
}

JBD_SELECT_SCHEMA = select.SELECT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(JbdSelect),
        cv.Optional(CONF_ICON, default="mdi:cog"): cv.icon,
        cv.Optional(CONF_OPTIONSMAP): ensure_option_map,
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = JBD_BMS_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_READ_EEPROM_REGISTER): JBD_SELECT_SCHEMA,
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_JBD_BMS_BLE_ID])

    for key in SELECTS:
        if key in config:
            conf = config[key]
            options_map = conf[CONF_OPTIONSMAP]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await select.register_select(var, conf, options=list(options_map.values()))
            cg.add(var.set_select_mappings(list(options_map.keys())))

            cg.add(getattr(hub, f"set_{key}_select")(var))
            cg.add(var.set_parent(hub))
