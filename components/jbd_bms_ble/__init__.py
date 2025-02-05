import esphome.codegen as cg
from esphome.components import ble_client
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@syssi"]

AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_JBD_BMS_BLE_ID = "jbd_bms_ble_id"

jbd_bms_ble_ns = cg.esphome_ns.namespace("jbd_bms_ble")
JbdBmsBle = jbd_bms_ble_ns.class_(
    "JbdBmsBle", ble_client.BLEClientNode, cg.PollingComponent
)

JBD_BMS_BLE_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_JBD_BMS_BLE_ID): cv.use_id(JbdBmsBle),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JbdBmsBle),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("2s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
