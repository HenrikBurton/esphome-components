#include "sx126x_spi.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cinttypes>

namespace esphome {
    namespace sx126x_spi {

        static const char *const TAG = "sx126x_spi";

        void Sx126XSpiComponent::setup() {
            if (this->cs_pin_ != nullptr) {
                this->cs_pin_->setup();
                this->cs_pin_->digital_write(true);
                ESP_LOGD(TAG, "CS pin setup!");
            }

            ESP_LOGD(TAG, "Setting up SPI interface to SX126X...");
            this->spi_setup();
            ESP_LOGD(TAG, "SPI interface setup!");

            if (this->busy_pin_ != nullptr) {
                this->busy_pin_->setup();
                ESP_LOGD(TAG, "BUSY pin setup!");
            }

            if (this->led_pin_ != nullptr) {
                this->led_pin_->setup();
                this->led_pin_->digital_write(false);
                this->led_on_ = false;
                ESP_LOGD(TAG, "LED pin setup!");
            }
        }

        void Sx126XSpiComponent::loop() {
            this->led_handler();

            if ((millis() - this->led_on_millis_) >= 1000) {
                ESP_LOGD(TAG, "Blink!");
                this->led_blink();

                ESP_LOGD(TAG, "Read status");
                uint8_t value = 0;
                this->cs_pin_->digital_write(false); //enable();
                this->write_byte16(0xC000);
                //this->write_byte(0x00);
                value = this->read_byte() << 8;
                value |= this->read_byte();
                // According to datsheet, the chip select must be written HIGH, LOW, HIGH
                // to correctly end the READ command.
                //this->cs_pin_->digital_write(true);
                //this->cs_pin_->digital_write(false);
                this->cs_pin_->digital_write(true); //this->disable();
                ESP_LOGD(TAG, "read_register_: %d", value);
            }

//            if (this->led_on_) {
//                this->cs_pin_->digital_write(true);
//            } else {
//                this->cs_pin_->digital_write(false);
//            }
        }

        void Sx126XSpiComponent::dump_config() {
            ESP_LOGCONFIG(TAG, "sx126x device");
            ESP_LOGCONFIG(TAG, "  frequency: %f", this->rf_frequency_);

            if (this->led_pin_ != nullptr) {
                ESP_LOGCONFIG(TAG, "  LED:");
                LOG_PIN("    Pin: ", this->led_pin_);
                ESP_LOGCONFIG(TAG, "    Duration: %d ms", this->led_blink_time_);
            } else {
                ESP_LOGCONFIG(TAG, "   No LED");
            }

            if (this->cs_pin_ != nullptr) {
                ESP_LOGCONFIG(TAG, "  CS:");
                LOG_PIN("    Pin: ", this->cs_pin_);
            } else {
                ESP_LOGCONFIG(TAG, "   No CS pin");
            }
            
            if (this->busy_pin_ != nullptr) {
                ESP_LOGCONFIG(TAG, "  BUSY:");
                LOG_PIN("    Pin: ", this->busy_pin_);
            } else {
                ESP_LOGCONFIG(TAG, "   No BUSY pin");
            }
        }

        float Sx126XSpiComponent::get_setup_priority() const { return setup_priority::DATA; }

        void Sx126XSpiComponent::led_blink() {
            if (this->led_pin_ != nullptr) {
              if (!this->led_on_) {
                this->led_on_millis_ = millis();
                this->led_pin_->digital_write(true);
                this->led_on_ = true;
              }
            }
          }
        
          void Sx126XSpiComponent::led_handler() {
            if (this->led_pin_ != nullptr) {
              if (this->led_on_) {
                if ((millis() - this->led_on_millis_) >= this->led_blink_time_) {
                  this->led_pin_->digital_write(false);
                  this->led_on_ = false;
                }
              }
            }
          }

    }  // namespace sx126x_spi
}  // namespace esphome
