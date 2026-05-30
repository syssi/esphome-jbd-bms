from esphome import pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_FLOW_CONTROL_PIN, CONF_ID

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

jbd_bms_up_ns = cg.esphome_ns.namespace("jbd_bms_up")
JbdBmsUp = jbd_bms_up_ns.class_("JbdBmsUp", cg.Component, uart.UARTDevice)
JbdBmsUpDevice = jbd_bms_up_ns.class_("JbdBmsUpDevice")

CONF_JBD_BMS_UP_ID = "jbd_bms_up_id"
CONF_RX_TIMEOUT = "rx_timeout"

CONFIG_SCHEMA = cv.All(
    cv.require_esphome_version(2025, 11, 0),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JbdBmsUp),
            cv.Optional(
                CONF_RX_TIMEOUT, default="150ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA),
)


async def to_code(config):
    cg.add_global(jbd_bms_up_ns.using)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
    if CONF_FLOW_CONTROL_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))


def jbd_bms_up_device_schema(default_address):
    schema = {cv.GenerateID(CONF_JBD_BMS_UP_ID): cv.use_id(JbdBmsUp)}
    if default_address is None:
        schema[cv.Required(CONF_ADDRESS)] = cv.int_range(min=1, max=247)
    else:
        schema[cv.Optional(CONF_ADDRESS, default=default_address)] = cv.int_range(
            min=1, max=247
        )
    return cv.Schema(schema)


async def register_jbd_bms_up_device(var, config):
    parent = await cg.get_variable(config[CONF_JBD_BMS_UP_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(parent.register_device(var))
