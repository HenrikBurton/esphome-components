#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
    namespace sx126x_spi {

        class Sx126XSpiComponent : public Component, 
                                   public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1KHZ> {

            public:
                void setup() override;
                void loop() override;
                void dump_config() override;

                void set_rf_frequency (float rf_frequency) { this->rf_frequency_ = rf_frequency; }
                void set_log_all(bool log_all) { this->log_all_ = log_all; }
                void set_busy_pin(GPIOPin *busy_pin) { this->busy_pin_ = busy_pin; }
                void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }

                float get_setup_priority() const override;

                void set_led_pin(GPIOPin *led) { this->led_pin_ = led; }
                void set_led_blink_time(uint32_t led_blink_time) { this->led_blink_time_ = led_blink_time; }

            protected:
                float rf_frequency_;
                bool log_all_;
                GPIOPin *busy_pin_{nullptr};
                GPIOPin *reset_pin_{nullptr};
                GPIOPin *led_pin_{nullptr};
                uint32_t led_blink_time_{0};
                uint32_t led_on_millis_{0};
                bool led_on_{false};
                void led_blink();
                void led_handler();
                void sx126xcommand(uint8_t*, uint8_t*, uint32_t);
                uint32_t sec_ticker{0};
                uint8_t rx_buffer[100];
        };

    }  // namespace sx126x_spi
}  // namespace esphome