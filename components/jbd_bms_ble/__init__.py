import esphome.codegen as cg
from esphome.components import ble_client
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PASSWORD

CONF_AUTH_TIMEOUT = "auth_timeout"

CODEOWNERS = ["@syssi"]

AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_JBD_BMS_BLE_ID = "jbd_bms_ble_id"

jbd_bms_ble_ns = cg.esphome_ns.namespace("jbd_bms_ble")
JbdBmsBle = jbd_bms_ble_ns.class_(
    "JbdBmsBle", ble_client.BLEClientNode, cg.PollingComponent
)


def validate_password(value):
    """Validate password according to JBD BMS requirements."""
    if not value:
        return value

    # Must be exactly 6 characters
    if len(value) != 6:
        raise cv.Invalid("Password must be exactly 6 characters long")

    # Only allow alphanumeric characters (0-9, a-z, A-Z)
    allowed_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    for char in value:
        if char not in allowed_chars:
            raise cv.Invalid(
                f"Password contains invalid character '{char}'. Only alphanumeric characters (0-9, a-z, A-Z) are allowed"
            )

    return value


JBD_BMS_BLE_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_JBD_BMS_BLE_ID): cv.use_id(JbdBmsBle),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JbdBmsBle),
            cv.Optional(CONF_PASSWORD, default=""): cv.All(
                cv.string_strict, validate_password
            ),
            cv.Optional(
                CONF_AUTH_TIMEOUT, default="10s"
            ): cv.positive_time_period_milliseconds,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("2s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    if CONF_PASSWORD in config and config[CONF_PASSWORD]:
        cg.add(var.set_password(config[CONF_PASSWORD]))

    if CONF_AUTH_TIMEOUT in config:
        cg.add(var.set_authentication_timeout(config[CONF_AUTH_TIMEOUT]))
