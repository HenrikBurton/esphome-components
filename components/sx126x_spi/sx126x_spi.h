#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
    namespace sx126x_spi {

        class Sx126XSpiComponent : public Component {

            public:
                void setup() override;
                void dump_config() override;

                float get_setup_priority() const override;

            protected:
        };

    }  // namespace sx126x_spi
}  // namespace esphome