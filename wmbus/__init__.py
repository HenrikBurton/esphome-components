import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import (
    CONF_ID
    CONF_MOSI_PIN,
    CONF_MISO_PIN,
    CONF_CLK_PIN,
    CONF_CS_PIN,
    CONF_NAME,
    CONF_FREQUENCY,
)

CODEOWNERS = ["@HenrikBurton"]
AUTO_LOAD = [ "sensor","text_sensor"]
DEPENDANCIES = ["spi"]

lora_sx126x_ns = cg.esphome_ns.namespace("wmbus_sx126x")

# empty_component_ns = cg.esphome_ns.namespace('empty_component')
wmbusSX126X = wmbus_sx126x_ns.class_('wmbusSX126X', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(wmbusSX126X),
    cv.Optional('name'): cv.string,

    cv.Optional(CONF_MOSI_PIN,       default=13):      pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_MISO_PIN,       default=12):      pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_CLK_PIN,        default=14):      pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_CS_PIN,         default=2):       pins.internal_gpio_output_pin_schema,
    cv.Optional(CONF_GDO0_PIN,       default=5):       pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_GDO2_PIN,       default=4):       pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_FREQUENCY,      default=868.950): cv.float_range(min=300, max=928),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_pin_lora_reset(config['pin_lora_reset']))
    cg.add(var.set_pin_lora_dio_1(config['pin_lora_dio_1']))
    cg.add(var.set_pin_lora_busy(config['pin_lora_busy']))
    cg.add(var.set_pin_lora_nss(config['pin_lora_nss']))
    cg.add(var.set_pin_lora_sclk(config['pin_lora_sclk']))
    cg.add(var.set_pin_lora_miso(config['pin_lora_miso']))
    cg.add(var.set_pin_lora_mosi(config['pin_lora_mosi']))
    cg.add(var.set_radio_txen(config['radio_txen']))
    cg.add(var.set_radio_rxen(config['radio_rxen']))

    cg.add(var.set_rf_frequency(config['rf_frequency']))

    cg.add(var.set_tx_output_power(config['tx_output_power']))
    cg.add(var.set_lora_bandwidth(config['lora_bandwidth']))
    cg.add(var.set_lora_spreading_factor(config['lora_spreading_factor']))
    cg.add(var.set_lora_codingrate(config['lora_codingrate']))
    cg.add(var.set_lora_preamble_length(config['lora_preamble_length']))
    cg.add(var.set_lora_symbol_timeout(config['lora_symbol_timeout']))
    cg.add(var.set_lora_fix_length_payload_on(config['lora_fix_length_payload_on']))
    cg.add(var.set_lora_iq_inversion_on(config['lora_iq_inversion_on']))
    cg.add(var.set_rx_timeout_value(config['rx_timeout_value']))
    cg.add(var.set_tx_timeout_value(config['tx_timeout_value']))
