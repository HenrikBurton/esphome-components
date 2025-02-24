#include "sx126x_spi.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cinttypes>

namespace esphome {
namespace sx126x_spi {

static const char *const TAG = "sx126x_spi";

void sx126x_spiComponent::setup() {
  ESP_LOGD(TAG, "Setting up SX126X...");
  this->spi_setup();
  ESP_LOGCONFIG(TAG, "SX1261 started!");
}

void sx126x_spiComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "SPIDevice");
  LOG_PIN("  CS pin: ", this->cs_);
  ESP_LOGCONFIG(TAG, "  Mode: %d", this->mode_);
  if (this->data_rate_ < 1000000) {
    ESP_LOGCONFIG(TAG, "  Data rate: %" PRId32 "kHz", this->data_rate_ / 1000);
  } else {
    ESP_LOGCONFIG(TAG, "  Data rate: %" PRId32 "MHz", this->data_rate_ / 1000000);
  }
}

float sx126x_spiComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace sx126x_spi
}  // namespace esphome
