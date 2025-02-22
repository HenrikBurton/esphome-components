import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_MOSI_PIN,
    CONF_MISO_PIN,
    CONF_CLK_PIN,
    CONF_CS_PIN,
    CONF_NAME,
    CONF_FREQUENCY,
    ONF_BIT_ORDER,
)

CODEOWNERS = ["@HenrikBurton"]
AUTO_LOAD = [ "sensor","text_sensor"]
DEPENDANCIES = ["spi"]

wmbus_sx126x_ns = cg.esphome_ns.namespace("wmbus_sx126x")

# empty_component_ns = cg.esphome_ns.namespace('empty_component')
wmbus_sx126x = wmbus_sx126x_ns.class_('wwmbus_sx126x', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(wmbusSX126X),
    cv.Optional('name'): cv.string,

    cv.Optional(CONF_MOSI_PIN,       default=13):          pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_MISO_PIN,       default=12):          pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_CLK_PIN,        default=14):          pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_CS_PIN,         default=2):           pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_GDO0_PIN,       default=5):           pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_GDO2_PIN,       default=4):           pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_FREQUENCY,      default=868.950):     cv.float_range(min=300, max=928),
    cv.Optional(CONF_BIT_ORDER,      default="msb_first"): cv.enum(ORDERS, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)

    cg.add(var.set_rf_frequency(config['rf_frequency']))
