#include "sx126x_spi.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cinttypes>

namespace esphome {
    namespace sx126x_spi {

        static const char *const TAG = "sx126x_spi";

        void Sx126XSpiComponent::setup() {
            ESP_LOGD(TAG, "Setting up SX126X...");
//            this->spi_setup();
            ESP_LOGCONFIG(TAG, "SX1261 started!");
        }

        void Sx126XSpiComponent::dump_config() {
            ESP_LOGCONFIG(TAG, "sx126x device");
            ESP_LOGCONFIG(TAG, "  frequency: %f", this->rf_frequency_);
//            ESP_LOGCONFIG(TAG, "  Mode: %d", this->mode_);
//            if (this->data_rate_ < 1000000) {
//                ESP_LOGCONFIG(TAG, "  Data rate: %" PRId32 "kHz", this->data_rate_ / 1000);
//            } else {
//                ESP_LOGCONFIG(TAG, "  Data rate: %" PRId32 "MHz", this->data_rate_ / 1000000);
//            }
            if (this->led_pin_ != nullptr) {
                ESP_LOGCONFIG(TAG, "  LED:");
                LOG_PIN("    Pin: ", this->led_pin_);
                ESP_LOGCONFIG(TAG, "    Duration: %d ms", this->led_blink_time_);
            } else {
                ESP_LOGCONFIG(TAG, "   No LED");
            }
        }

        float Sx126XSpiComponent::get_setup_priority() const { return setup_priority::DATA; }

    }  // namespace sx126x_spi
}  // namespace esphome
