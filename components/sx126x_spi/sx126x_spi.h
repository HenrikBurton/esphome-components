#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace sx126x_spi {

class sx126x_spiComponent : public Component,
                             public spi::wmbusSX126X<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH,
                                                 spi::CLOCK_PHASE_TRAILING, spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override;

 protected:
};

}  // namespace sx126x_spi
}  // namespace esphome