import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_FREQUENCY
)

CONF_LOG_ALL = "log_all"

CODEOWNERS = ["@HenrikBurton"]
AUTO_LOAD = [ "sensor","text_sensor"]
DEPENDANCIES = ["spi"]

sx126x_spi_ns = cg.esphome_ns.namespace("sx126x_spi")

# empty_component_ns = cg.esphome_ns.namespace('empty_component')
Sx126XSpiComponent = sx126x_spi_ns.class_('Sx126XSpi', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Sx126XSpiComponent),
    cv.Optional('name'): cv.string,

    cv.Optional(CONF_FREQUENCY,      default=868.950):     cv.float_range(min=300, max=928),
    cv.Optional(CONF_LOG_ALL,        default=False):       cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_rf_frequency(config[CONF_FREQUENCY]))
    cg.add(var.set_log_all(config[CONF_LOG_ALL]))
