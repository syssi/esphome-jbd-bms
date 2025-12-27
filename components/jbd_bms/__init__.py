import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UART_ID, CONF_PROTOCOL, CONF_ADDRESS, CONF_TX_PIN, CONF_RX_PIN

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
            cv.Optional(CONF_TX_PIN): uart.pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_RX_PIN): uart.pins.internal_gpio_input_pin_schema,
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
            MASTERS[config[CONF_UART_ID]] = [var]
            master = var
            prev = None
        else:
            master = MASTERS[config[CONF_UART_ID]][0]
            prev = MASTERS[config[CONF_UART_ID]][-1]
        cg.add(var.set_battery_address(config[CONF_ADDRESS]))
        if prev:
            cg.add(prev.set_next_battery(var))
        cg.add(var.set_master(master))
        tx_pin = await cg.gpio_pin_expression(config[CONF_TX_PIN])
        cg.add(var.set_tx_pin(tx_pin))
        rx_pin = await cg.gpio_pin_expression(config[CONF_RX_PIN])
        cg.add(var.set_rx_pin(rx_pin))
