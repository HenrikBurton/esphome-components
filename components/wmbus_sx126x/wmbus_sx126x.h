#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace wmbus_sx126x {

class wmbusSX126XComponent : public Component,
                             public spi::wmbusSX126<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH,
                                                 spi::CLOCK_PHASE_TRAILING, spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override;

 protected:
};

}  // namespace wmbus_sx126x
}  // namespace esphome