import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UART_ID, CONF_PROTOCOL, CONF_ADDRESS

CODEOWNERS = ["@syssi"]

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_JBD_BMS_ID = "jbd_bms_id"
CONF_RX_TIMEOUT = "rx_timeout"

jbd_bms_ns = cg.esphome_ns.namespace("jbd_bms")
JbdBms = jbd_bms_ns.class_("JbdBms", cg.PollingComponent, uart.UARTDevice)
JbdBmsUP = jbd_bms_ns.class_("JbdBmsUP", JbdBms)

MASTERS = {}

JBD_BMS_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_JBD_BMS_ID): cv.use_id(JbdBms),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JbdBms),
            cv.Optional(
                CONF_RX_TIMEOUT, default="150ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_PROTOCOL,
                default="JBD",
            ): cv.enum(
                {
                    "JBD": 0,
                    "UP": 1,
                },
                upper=True,
            ),
            cv.Optional(CONF_ADDRESS, default=1): cv.int_range(0, 16),
        }
    )
    .extend(cv.polling_component_schema("2s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    if config[CONF_PROTOCOL] == "UP":
        config[CONF_ID].type = JbdBmsUP
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
    if config[CONF_PROTOCOL] == "UP":
        if config[CONF_UART_ID] not in MASTERS:
            MASTERS[config[CONF_UART_ID]] = var
            cg.add(var.set_is_master(True))
        cg.add(var.set_battery_address(config[CONF_ADDRESS]))
        cg.add(MASTERS[config[CONF_UART_ID]].add_battery(var))
