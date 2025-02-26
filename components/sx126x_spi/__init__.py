import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_BUSY_PIN
)


CONF_RF_FREQUENCY = "rf_frequency"
CONF_LOG_ALL = "log_all"
CONF_LED_PIN = "led_pin"
CONF_LED_BLINK_TIME = "led_blink_time"

CODEOWNERS = ["@HenrikBurton"]
AUTO_LOAD = [ "sensor","text_sensor"]
DEPENDANCIES = ["spi"]

sx126x_spi_ns = cg.esphome_ns.namespace("sx126x_spi")

# empty_component_ns = cg.esphome_ns.namespace('empty_component')
SX126XSPI = sx126x_spi_ns.class_("Sx126XSpiComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID():                                       cv.declare_id(SX126XSPI),
    cv.Optional('name'):                                   cv.string,

    cv.Optional(CONF_RF_FREQUENCY,   default=868.950):     cv.float_range(min=300, max=928),
    cv.Optional(CONF_BUSY_PIN):                            pins.gpio_output_pin_schema,
    cv.Optional(CONF_LOG_ALL,        default=False):       cv.boolean,

    cv.Optional(CONF_LED_PIN):                             pins.gpio_output_pin_schema,
    cv.Optional(CONF_LED_BLINK_TIME, default="200ms"):     cv.positive_time_period,
}).extend(cv.COMPONENT_SCHEMA)
  .extend(spi.spi_device_schema(cs_pin_required=True))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)

    cg.add(var.set_rf_frequency(config[CONF_RF_FREQUENCY]))
    cg.add(var.set_log_all(config[CONF_LOG_ALL]))

    if CONF_BUSY_PIN in config:
        busy_pin = await cg.gpio_pin_expression(config[CONF_BUSY_PIN])
        cg.add(var.set_busy_pin(busy_pin))

    if CONF_LED_PIN in config:
        led_pin = await cg.gpio_pin_expression(config[CONF_LED_PIN])
        cg.add(var.set_led_pin(led_pin))
        cg.add(var.set_led_blink_time(config[CONF_LED_BLINK_TIME].total_milliseconds))
