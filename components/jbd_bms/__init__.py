from esphome import pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_FLOW_CONTROL_PIN, CONF_ID
from esphome.cpp_helpers import gpio_pin_expression

CODEOWNERS = ["@syssi"]

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_JBD_BMS_ID = "jbd_bms_id"
CONF_RX_TIMEOUT = "rx_timeout"

jbd_bms_ns = cg.esphome_ns.namespace("jbd_bms")
JbdBms = jbd_bms_ns.class_("JbdBms", cg.PollingComponent, uart.UARTDevice)

JBD_BMS_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_JBD_BMS_ID): cv.use_id(JbdBms),
    }
)

CONFIG_SCHEMA = cv.All(
    cv.require_esphome_version(2025, 11, 0),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JbdBms),
            cv.Optional(
                CONF_RX_TIMEOUT, default="150ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.polling_component_schema("2s"))
    .extend(uart.UART_DEVICE_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
    if CONF_FLOW_CONTROL_PIN in config:
        pin = await gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))
