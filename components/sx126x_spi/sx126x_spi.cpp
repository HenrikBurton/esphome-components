#include "sx126x_spi.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cinttypes>

namespace esphome {
    namespace sx126x_spi {

        static const char *const TAG = "sx126x_spi";

        void Sx126XSpiComponent::setup() {
            ESP_LOGD(TAG, "Setting up SX126X...");
            this->spi_setup();
            ESP_LOGCONFIG(TAG, "SX1261 started!");
        }

        void Sx126XSpiComponent::dump_config() {
            ESP_LOGCONFIG(TAG, "SPIDevice");
            LOG_PIN("  CS pin: ", this->cs_);
            ESP_LOGCONFIG(TAG, "  Mode: %d", this->mode_);
            if (this->data_rate_ < 1000000) {
                ESP_LOGCONFIG(TAG, "  Data rate: %" PRId32 "kHz", this->data_rate_ / 1000);
            } else {
                ESP_LOGCONFIG(TAG, "  Data rate: %" PRId32 "MHz", this->data_rate_ / 1000000);
            }
        }

        float Sx126XSpiComponent::get_setup_priority() const { return setup_priority::DATA; }

    }  // namespace sx126x_spi
}  // namespace esphome
