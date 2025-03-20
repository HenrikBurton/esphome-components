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

            if (this->irq_pin_ != nullptr) {
              this->irq_pin_->setup();
              ESP_LOGD(TAG, "IRQ pin setup!");
            }

            if (this->led_pin_ != nullptr) {
                this->led_pin_->setup();
                this->led_pin_->digital_write(false);
                this->led_on_ = false;
                ESP_LOGD(TAG, "LED pin setup!");
            }

            this->sec_ticker = millis();

            // Setup device for FSK

            state = standby(RADIOLIB_SX126X_STANDBY_RC);

            //state = setTCXO(tcxoVoltage);

            state = setPacketType(RADIOLIB_SX126X_PACKET_TYPE_GFSK);

            state = setRfFrequency(868.950f);

            state = setBufferBaseAddress(0x00, 0x00);

            //state = setCadParams();

            //state = setCalibration(RADIOLIB_SX126X_CALIBRATE_ALL);

            // bitrate, freqDeviation, Bandwidth, PulseShape
            state = setModulationParams(32.768f, 50.0f, 156.2f, RADIOLIB_SX126X_GFSK_FILTER_NONE);

            state = setPacketParams(16, 
                                    RADIOLIB_SX126X_GFSK_PREAMBLE_DETECT_8, 
                                    //24, // Syncword length
                                    16,
                                    RADIOLIB_SX126X_GFSK_ADDRESS_FILT_OFF, 
                                    RADIOLIB_SX126X_GFSK_PACKET_VARIABLE, 
                                    0xf0,
                                    RADIOLIB_SX126X_GFSK_CRC_OFF, 
                                    RADIOLIB_SX126X_GFSK_WHITENING_OFF
                                  );
            
//            state = setDioIrqParams(RADIOLIB_SX126X_IRQ_RX_DONE, RADIOLIB_SX126X_IRQ_RX_DONE, 0, 0);
            state = setDioIrqParams(RADIOLIB_SX126X_IRQ_ALL, RADIOLIB_SX126X_IRQ_ALL, 0, 0);

            state = setSyncWord();

            state = setDIO3AsTCXOCtrl(RADIOLIB_SX126X_DIO3_OUTPUT_3_0, 64);  // Delay = 1 ms / 0.015625 ms = 64

            state = setFallbackMode(RADIOLIB_SX126X_RX_TX_FALLBACK_MODE_STDBY_XOSC);

            //state = setRegulatorMode(RADIOLIB_SX126X_REGULATOR_LDO); // RADIOLIB_SX126X_REGULATOR_DC_DC
           
            //state = setCurrentLimit(60.0);

            //state = clearIrqStatus(RADIOLIB_SX126X_IRQ_ALL);

            //state = setDio2AsRfSwitch(RADIOLIB_SX126X_DIO2_AS_RF_SWITCH);

            state = standby(RADIOLIB_SX126X_STANDBY_XOSC);

            state = setRx(0x000000);
            //state = setFs();
        }

        void Sx126XSpiComponent::loop() {
            this->led_handler();

            uint32_t irq = this->irq_pin_->digital_read();

            if(irq) {
              uint16_t irqStatus = getIrqStatus();

              ESP_LOGD(TAG, "IRQ: %04X", irqStatus);

              uint16_t status = clearIrqStatus(RADIOLIB_SX126X_IRQ_ALL);

              this->led_blink();

              if(irqStatus & RADIOLIB_SX126X_IRQ_RX_DONE) {
                  uint16_t rxBufferStatus = getRxBufferStatus();
                  ESP_LOGD(TAG, "Length and pointer: %04X", rxBufferStatus);

                  uint8_t cmd[] = { 0x1e, (uint8_t) (rxBufferStatus & 0xff), 0x00, 
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
                  sx126xcommand(cmd, this->rx_buffer, 13);

                  char string[200];
                  for(uint32_t i = 0; i < ((rxBufferStatus >> 8) & 0xff); i++) {
                    sprintf(string + i * 3, "%02x ", this->rx_buffer[i]);
                  }
                  string[i * 3] = '\0';

                  ESP_LOGD(TAG, "Rx: %s", string);
                  /*
                  ESP_LOGD(TAG, "status: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", this->rx_buffer[3], 
                                                                    this->rx_buffer[4],
                                                                    this->rx_buffer[5],
                                                                    this->rx_buffer[6],
                                                                    this->rx_buffer[7],
                                                                    this->rx_buffer[8],
                                                                    this->rx_buffer[9],
                                                                    this->rx_buffer[10],
                                                                    this->rx_buffer[11],
                                                                    this->rx_buffer[12]
                  );
                  */
              }

              status = getStatus();
              ESP_LOGD(TAG, "Status: %02X", status);

              if((status & 0x70) != RADIOLIB_SX126X_STATUS_MODE_RX) {
                status = setRx(0x000000);
              }

            }
/*
            if ((millis() - this->sec_ticker) >= 1000) {
                ESP_LOGD(TAG, "Blink!");
                this->sec_ticker = millis();
                this->led_blink();
                uint8_t cmd5[] = {0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };                        // GetStats
                sx126xcommand(cmd5, this->rx_buffer, 4);
                ESP_LOGD(TAG, "status: %02X, %04X, %04X, %04X", this->rx_buffer[1], 
                                                        this->rx_buffer[2] << 8 | this->rx_buffer[3],
                                                        this->rx_buffer[4] << 8 | this->rx_buffer[5],
                                                        this->rx_buffer[6] << 8 | this->rx_buffer[7]);
            }
*/
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
          
            if (this->irq_pin_ != nullptr) {
                ESP_LOGCONFIG(TAG, "  IRQ:");
                LOG_PIN("    Pin: ", this->irq_pin_);
            } else {
                ESP_LOGCONFIG(TAG, "   No IRQ pin");
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

          int16_t Sx126XSpiComponent::sx126xcommand(uint8_t *command, uint8_t *response, uint32_t length) {
            
            // Wait until device is not BUSY
            while(this->busy_pin_->digital_read()){
              delay(1);
            }
            this->delegate_->begin_transaction();
            this->delegate_->transfer(command, response, length);
            this->delegate_->end_transaction();
            return(0);
          }
           
          uint16_t Sx126XSpiComponent::getStatus() {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_GET_STATUS, 0x00 };
            uint16_t status = sx126xcommand(data, this->rx_buffer, 2);
            return((uint16_t)this->rx_buffer[1]);
          }

          int16_t Sx126XSpiComponent::standby(uint8_t mode) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_STANDBY, mode };
            return(sx126xcommand(data, this->rx_buffer, 2));
          }

          int16_t Sx126XSpiComponent::setRfFrequency(float freq) {

            uint32_t freqRaw = (freq * (uint32_t(1) << RADIOLIB_SX126X_DIV_EXPONENT)) / RADIOLIB_SX126X_CRYSTAL_FREQ;

            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_RF_FREQUENCY, 
              (uint8_t)((freqRaw >> 24) & 0xff),
              (uint8_t)((freqRaw >> 16) & 0xff),
              (uint8_t)((freqRaw >> 8) & 0xff),
              (uint8_t)(freqRaw & 0xff)
            };
            return(sx126xcommand(data, this->rx_buffer, 5));
          }

          int16_t Sx126XSpiComponent::setFs() {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_FS };
            return(sx126xcommand(data, this->rx_buffer, 1));
          }
          
          int16_t Sx126XSpiComponent::setRx(uint32_t timeout) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_RX, 
              (uint8_t)((timeout >> 16) & 0xff),
              (uint8_t)((timeout >> 8) & 0xff),
              (uint8_t)(timeout & 0xff)
            };
            return(sx126xcommand(data, this->rx_buffer, 4));
          }

          int16_t Sx126XSpiComponent::setBufferBaseAddress(uint8_t txBaseAddress, uint8_t rxBaseAddress) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_BUFFER_BASE_ADDRESS, txBaseAddress, rxBaseAddress };
            return(sx126xcommand(data, this->rx_buffer, 3));
          }

          uint16_t Sx126XSpiComponent::getRxBufferStatus() {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_GET_RX_BUFFER_STATUS, 0x00, 0x00, 0x00 };
            uint16_t status = sx126xcommand(data, this->rx_buffer, 4);
            return((uint16_t)(this->rx_buffer[2] << 8) | this->rx_buffer[3]);
          }

          int16_t Sx126XSpiComponent::setPacketType(uint8_t type) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_PACKET_TYPE, type };
            return(sx126xcommand(data, this->rx_buffer, 2));
          }

          int16_t Sx126XSpiComponent::setFallbackMode(uint8_t mode) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_RX_TX_FALLBACK_MODE, mode };
            return(sx126xcommand(data, this->rx_buffer, 2));
          }

          int16_t Sx126XSpiComponent::setCadParams() {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_CAD_PARAMS,
                               RADIOLIB_SX126X_CAD_ON_8_SYMB,
                               9 + 13,
                               RADIOLIB_SX126X_CAD_PARAM_DET_MIN,
                               RADIOLIB_SX126X_CAD_GOTO_STDBY,
                               0x00,
                               0x00,
                               0x00
            };

            return(sx126xcommand(data, this->rx_buffer, 8));
          }

          uint16_t Sx126XSpiComponent::getIrqStatus() {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_GET_IRQ_STATUS, 0x00, 0x00, 0x00 };
            uint16_t status = sx126xcommand(data, this->rx_buffer, 4);
            return((uint16_t)(this->rx_buffer[2] << 8) | this->rx_buffer[3]);
          }

          int16_t Sx126XSpiComponent::clearIrqStatus(uint16_t clearIrqParams) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_CLEAR_IRQ_STATUS, (uint8_t)((clearIrqParams >> 8) & 0xFF), (uint8_t)(clearIrqParams & 0xFF) };
            return(sx126xcommand(data, this->rx_buffer, 3));
          }

          int16_t Sx126XSpiComponent::setDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask) {
            uint8_t data[] = {RADIOLIB_SX126X_CMD_SET_DIO_IRQ_PARAMS,
                               (uint8_t)((irqMask >> 8) & 0xFF), (uint8_t)(irqMask & 0xFF),
                               (uint8_t)((dio1Mask >> 8) & 0xFF), (uint8_t)(dio1Mask & 0xFF),
                               (uint8_t)((dio2Mask >> 8) & 0xFF), (uint8_t)(dio2Mask & 0xFF),
                               (uint8_t)((dio3Mask >> 8) & 0xFF), (uint8_t)(dio3Mask & 0xFF)
            };
            return(sx126xcommand(data, this->rx_buffer, 9));
          }
          
          int16_t Sx126XSpiComponent::setDIO3AsTCXOCtrl(uint8_t tcxoVoltage, uint32_t delay) {
            uint8_t data[] = {RADIOLIB_SX126X_CMD_SET_DIO3_AS_TCXO_CTRL,
                               tcxoVoltage,
                               (uint8_t)((delay >> 24) & 0xFF), (uint8_t)((delay >> 16) & 0xFF), (uint8_t)(delay & 0xff)
            };
            return(sx126xcommand(data, this->rx_buffer, 5));
          }

          int16_t Sx126XSpiComponent::setCalibration(uint8_t type) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_CALIBRATE, type };
            return(sx126xcommand(data, this->rx_buffer, 2));
          }

          int16_t Sx126XSpiComponent::setRegulatorMode(uint8_t mode) {
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_REGULATOR_MODE, mode };
            return(sx126xcommand(data, this->rx_buffer, 2));
          }

          int16_t Sx126XSpiComponent::setModulationParams(float br, float freqDev, float rxBw, uint32_t pulseShape) {

            // calculate raw bit rate value
            uint32_t brRaw = (uint32_t)((RADIOLIB_SX126X_CRYSTAL_FREQ * 1000000.0f * 32.0f) / (br * 1000.0f));

            // calculate raw frequency deviation value
            uint32_t freqDevRaw = (uint32_t)(((freqDev * 1000.0f) * (float)((uint32_t)(1) << 25)) / (RADIOLIB_SX126X_CRYSTAL_FREQ * 1000000.0f));

            // check allowed receiver bandwidth values

            uint8_t rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_467_0;

            if(fabsf(rxBw - 4.8f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_4_8;
            } else if(fabsf(rxBw - 5.8f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_5_8;
            } else if(fabsf(rxBw - 7.3f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_7_3;
            } else if(fabsf(rxBw - 9.7f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_9_7;
            } else if(fabsf(rxBw - 11.7f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_11_7;
            } else if(fabsf(rxBw - 14.6f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_14_6;
            } else if(fabsf(rxBw - 19.5f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_19_5;
            } else if(fabsf(rxBw - 23.4f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_23_4;
            } else if(fabsf(rxBw - 29.3f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_29_3;
            } else if(fabsf(rxBw - 39.0f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_39_0;
            } else if(fabsf(rxBw - 46.9f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_46_9;
            } else if(fabsf(rxBw - 58.6f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_58_6;
            } else if(fabsf(rxBw - 78.2f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_78_2;
            } else if(fabsf(rxBw - 93.8f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_93_8;
            } else if(fabsf(rxBw - 117.3f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_117_3;
            } else if(fabsf(rxBw - 156.2f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_156_2;
            } else if(fabsf(rxBw - 187.2f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_187_2;
            } else if(fabsf(rxBw - 234.3f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_234_3;
            } else if(fabsf(rxBw - 312.0f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_312_0;
            } else if(fabsf(rxBw - 373.6f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_373_6;
            } else if(fabsf(rxBw - 467.0f) <= 0.001f) {
              rxBandwidth = RADIOLIB_SX126X_GFSK_RX_BW_467_0;
            }
          
            uint8_t data[] = { RADIOLIB_SX126X_CMD_SET_MODULATION_PARAMS,
              (uint8_t)((brRaw >> 16) & 0xFF), (uint8_t)((brRaw >> 8) & 0xFF), (uint8_t)(brRaw & 0xFF),
              (uint8_t) pulseShape, (uint8_t) rxBandwidth,
              (uint8_t)((freqDevRaw >> 16) & 0xFF), (uint8_t)((freqDevRaw >> 8) & 0xFF), (uint8_t)(freqDevRaw & 0xFF)
            };

            return(sx126xcommand(data, this->rx_buffer, 9));
          }

          int16_t Sx126XSpiComponent::setCurrentLimit(float currentLimit) {

            // calculate raw value
            uint8_t rawLimit = (uint8_t)(currentLimit / 2.5f);
          
            // update register
            uint8_t data[] = { 
              RADIOLIB_SX126X_CMD_WRITE_REGISTER,
              RADIOLIB_SX126X_REG_OCP_CONFIGURATION >> 8 & 0xff, RADIOLIB_SX126X_REG_OCP_CONFIGURATION & 0xff,
              rawLimit
            };

            return(sx126xcommand(data, this->rx_buffer, 4));
          }

          int16_t Sx126XSpiComponent::setPacketParams(uint16_t preambleLen, 
                                                      uint8_t preambleDetectorLen, 
                                                      uint8_t syncWordLen, 
                                                      uint8_t addrComp, 
                                                      uint8_t packType, 
                                                      uint8_t payloadLen,
                                                      uint8_t crcType, 
                                                      uint8_t whiten
                                                    ) {

            uint8_t data[] = {
              RADIOLIB_SX126X_CMD_SET_PACKET_PARAMS,
              (uint8_t)((preambleLen >> 8) & 0xFF), (uint8_t)(preambleLen & 0xFF),
              preambleDetectorLen, syncWordLen, addrComp,
              packType, payloadLen, crcType, whiten
            };
            return(sx126xcommand(data, this->rx_buffer, 10));
          }

          int16_t Sx126XSpiComponent::setSyncWord() {
          
            // update register
            uint8_t data[] = { 
              RADIOLIB_SX126X_CMD_WRITE_REGISTER,
              RADIOLIB_SX126X_REG_SYNC_WORD_0 >> 8 & 0xff, RADIOLIB_SX126X_REG_SYNC_WORD_0  & 0xff,
              0x54, 0x76, 0x96, 0x00, 0x00, 0x00
            };

            return(sx126xcommand(data, this->rx_buffer, 9));
          }

          int16_t Sx126XSpiComponent::setDio2AsRfSwitch(uint8_t dio2mode) {

            uint8_t data[] = {
              RADIOLIB_SX126X_CMD_SET_DIO2_AS_RF_SWITCH_CTRL,
              dio2mode
            };

            return(sx126xcommand(data, this->rx_buffer, 2));
          }

        }  // namespace sx126x_spi
}  // namespace esphome
