#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
    namespace sx126x_spi {

        class Sx126XSpiComponent : public Component {

            public:
                void setup() override;
                void dump_config() override;

                void set_rf_frequency (uint32_t rf_frequency) { this->rf_frequency_ = rf_frequency; }
                void set_log_all(bool log_all) { this->log_all_ = log_all; }
                
                float get_setup_priority() const override;

            protected:
                uint32_t rf_frequency_;
                bool log_all_;
        };

    }  // namespace sx126x_spi
}  // namespace esphome