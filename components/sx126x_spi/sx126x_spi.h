#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
    namespace sx126x_spi {

        class Sx126XSpiComponent : public Component,
                                   public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, spi::CLOCK_PHASE_LEADING,
                                                         spi::DATA_RATE_1MHZ> {

            public:
                void setup() override;
                void dump_config() override;

                void set_rf_frequency (float rf_frequency) { this->rf_frequency_ = rf_frequency; }
                void set_log_all(bool log_all) { this->log_all_ = log_all; }
                
                float get_setup_priority() const override;

            protected:
                float rf_frequency_;
                bool log_all_;
        };

    }  // namespace sx126x_spi
}  // namespace esphome