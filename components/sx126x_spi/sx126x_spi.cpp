#include "sx126x_spi.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cinttypes>

namespace esphome {
    namespace sx126x_spi {

        static const char *const TAG = "sx126x_spi";

        void Sx126XSpiComponent::setup() {
            ESP_LOGD(TAG, "Setting up SPI interface to SX126X...");
            this->spi_setup();
            ESP_LOGD(TAG, "SPI interface setup!");

            if (this->busy_pin_ != nullptr) {
                this->busy_pin_->setup();
                ESP_LOGD(TAG, "BUSY pin setup!");
            }

            if (this->reset_pin_ != nullptr) {
              this->reset_pin_->setup();
              this->reset_pin_->digital_write(true);
              delay(2);
              this->reset_pin_->digital_write(false);
              delay(2);
              this->reset_pin_->digital_write(true);
              ESP_LOGD(TAG, "RESET pin setup!");
            }

            if (this->led_pin_ != nullptr) {
                this->led_pin_->setup();
                this->led_pin_->digital_write(false);
                this->led_on_ = false;
                ESP_LOGD(TAG, "LED pin setup!");
            }

            this->sec_ticker = millis();

            delay(3);
            uint8_t cmd1[] = {0x8A, 0x00};                       // Set packet type with protocol GFSK
            sx126xcommand(cmd1, this->rx_buffer, 2);
            uint8_t cmd2[] = {0x93, 0x20};                       // SetRxTxFallbackMode,  The radio goes into STDBY_RC mode after Tx or Rx
            sx126xcommand(cmd2, this->rx_buffer, 2);
            uint8_t cmd3[] = {0x88, 0x03, 0x16, 0x0A, 0x00, 0x00, 0x00, 0x00};   // SetCadParams
            sx126xcommand(cmd3, this->rx_buffer, 8);
            uint8_t cmd4[] = {0x89, 0b01111111};                  // Calibrate, All
            //sx126xcommand(cmd4, this->rx_buffer, 2);
            delay(3);
            uint8_t cmd6[] = {0x8B, 0x00, 0x28, 0x00, 0x00, 0x1A, 0x00, 0x02, 0x75 };      // SetModulationParams, br = 100000, PulseShape = non, Bw = 156,2, FreqDev = 600
            sx126xcommand(cmd6, this->rx_buffer, 9);
            uint8_t cmd8[] = {0x8C, 0x00, 0x20, 0x00, 0x10, 0x00, 0x01, 0xFF, 0x01, 0x00}; // SetPacketParams, 
            sx126xcommand(cmd8, this->rx_buffer, 10);
            uint8_t cmd7[] = {0x82, 0xff, 0xff, 0xff};                        // SetRX, no timeout
            sx126xcommand(cmd7, this->rx_buffer, 4);
            delay(3);
            uint8_t cmd5[] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };                        // GetStats
            sx126xcommand(cmd5, this->rx_buffer, 2);
            sx126xcommand(cmd5, this->rx_buffer, 2);
            uint32_t value;
            value = this->rx_buffer[2] << 8 | this->rx_buffer[3];
            ESP_LOGD(TAG, "status: %d NbPktRX %d", this->rx_buffer[0] << 8 | this->rx_buffer[1], this->rx_buffer[2] << 8 | this->rx_buffer[3]);
        }

        void Sx126XSpiComponent::loop() {
            this->led_handler();

            if ((millis() - this->sec_ticker) >= 1000) {
                this->sec_ticker = millis();
                ESP_LOGD(TAG, "Blink!");
                this->led_blink();
                uint8_t cmd5[] = {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };                        // GetStats
                sx126xcommand(cmd5, this->rx_buffer, 2);
                uint32_t value;
                value = this->rx_buffer[2] << 8 | this->rx_buffer[3];
                ESP_LOGD(TAG, "status: %d NbPktRX %d", this->rx_buffer[0] << 8 | this->rx_buffer[1], this->rx_buffer[2] << 8 | this->rx_buffer[3]);
            }
        }

        void Sx126XSpiComponent::dump_config() {
            ESP_LOGCONFIG(TAG, "sx126x device");
            ESP_LOGCONFIG(TAG, "  radio frequency: %f", this->rf_frequency_);

            if (this->led_pin_ != nullptr) {
                ESP_LOGCONFIG(TAG, "  LED:");
                LOG_PIN("    Pin: ", this->led_pin_);
                ESP_LOGCONFIG(TAG, "    Duration: %d ms", this->led_blink_time_);
            } else {
                ESP_LOGCONFIG(TAG, "   No LED");
            }

            if (this->busy_pin_ != nullptr) {
                ESP_LOGCONFIG(TAG, "  BUSY:");
                LOG_PIN("    Pin: ", this->busy_pin_);
            } else {
                ESP_LOGCONFIG(TAG, "   No BUSY pin");
            }

            if (this->reset_pin_ != nullptr) {
              ESP_LOGCONFIG(TAG, "  RESET:");
              LOG_PIN("    Pin: ", this->reset_pin_);
          } else {
              ESP_LOGCONFIG(TAG, "   No RESET pin");
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

          void Sx126XSpiComponent::sx126xcommand(uint8_t *command, uint8_t *response, uint32_t length) {
            this->delegate_->begin_transaction();
            this->delegate_->transfer(command, response, length);
            this->delegate_->end_transaction();
          }

    }  // namespace sx126x_spi
}  // namespace esphome
