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

            // Setup device for FSK

            state = standby(RADIOLIB_SX126X_STANDBY_RC);

            //state = setTCXO(tcxoVoltage);

            state = setPacketType(RADIOLIB_SX126X_PACKET_TYPE_GFSK);

            state = setRfFrequency(868.950f);

            state = setFallbackMode(RADIOLIB_SX126X_RX_TX_FALLBACK_MODE_STDBY_RC);

            state = setBufferBaseAddress(0x00, 0x00);

            //state = setCadParams();

            //state = setCalibration(RADIOLIB_SX126X_CALIBRATE_ALL);

            // bitrate, freqDeviation, Bandwidth, PulseShape
            state = setModulationParams(32.768f, 50.0f, 156.2f, RADIOLIB_SX126X_GFSK_FILTER_NONE);

            state = setPacketParams(16, 
                                    RADIOLIB_SX126X_GFSK_PREAMBLE_DETECT_8, 
                                    24, 
                                    RADIOLIB_SX126X_GFSK_ADDRESS_FILT_OFF, 
                                    RADIOLIB_SX126X_GFSK_PACKET_VARIABLE, 
                                    0xf0,
                                    RADIOLIB_SX126X_GFSK_CRC_OFF, 
                                    RADIOLIB_SX126X_GFSK_WHITENING_OFF
                                  );
            
            state = setDioIrqParams(RADIOLIB_SX126X_IRQ_RX_DONE, RADIOLIB_SX126X_IRQ_RX_DONE, 0, 0);

            state = setSyncWord();

            //state = setRegulatorMode(RADIOLIB_SX126X_REGULATOR_LDO); // RADIOLIB_SX126X_REGULATOR_DC_DC
           
            //state = setCurrentLimit(60.0);

            //state = clearIrqStatus(RADIOLIB_SX126X_IRQ_ALL);

            //state = setDio2AsRfSwitch(RADIOLIB_SX126X_DIO2_AS_RF_SWITCH);

            state = setRx(0xffffff);



/*
            delay(3);
            uint8_t cmd0[] = {0xC0, 0x00 };                      // GetStats
            sx126xcommand(cmd0, this->rx_buffer, 2);
            uint8_t cmd12[] = {0x1D, 0x06, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };      // Read syncword                // ReadRegister SyncWord
            sx126xcommand(cmd12, this->rx_buffer, 12);
            uint8_t cmd16[] = {0x0D, 0x06, 0xc0, 0x54, 0x76, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00 };      // Write syncword
            sx126xcommand(cmd16, this->rx_buffer, 11);
            sx126xcommand(cmd12, this->rx_buffer, 12);
            uint8_t cmd13[] = {0x9D, 0x01 };                      // SetDio2AsRfSwitchCtrl, enable
            sx126xcommand(cmd13, this->rx_buffer, 2);
            uint8_t cmd15[] = {0x8F, 0x00, 0x00 };                      // SetBufferBaseAddress
            sx126xcommand(cmd15, this->rx_buffer, 3);
            uint8_t cmd9[] = {0x80, 0x00};                       // SetStandby, STDBY_RC = 0x00 STDBY_XOSC = 0x01
            sx126xcommand(cmd9, this->rx_buffer, 2);
            delay(3);
            uint32_t freq = uint32_t ((868.0f * (1 << 25)) / 32.0f);
            uint8_t cmd10[] = {0x86, uint8_t (freq >> 24 & 0xff), 
                                     uint8_t (freq >> 16 & 0xff),
                                     uint8_t (freq >> 8 & 0xff),
                                     uint8_t (freq & 0xff) };         // Set RF requency
            sx126xcommand(cmd10, this->rx_buffer, 5);
            uint8_t cmd1[] = {0x8A, 0x00};                       // Set packet type with protocol GFSK
            sx126xcommand(cmd1, this->rx_buffer, 2);
            uint8_t cmd2[] = {0x93, 0x20};                       // SetRxTxFallbackMode,  The radio goes into STDBY_RC mode after Tx or Rx
            sx126xcommand(cmd2, this->rx_buffer, 2);
            uint8_t cmd3[] = {0x88, 0x03, 0x16, 0x0A, 0x00, 0x00, 0x00, 0x00};   // SetCadParams
            //sx126xcommand(cmd3, this->rx_buffer, 8);
            uint8_t cmd11[] = {0x98, 0xD7, 0xDB };                // Calibrate image
            //sx126xcommand(cmd11, this->rx_buffer, 3);
            delay(3);
            uint8_t cmd4[] = {0x89, 0b01111111};                  // Calibrate, All
            //sx126xcommand(cmd4, this->rx_buffer, 2);
            delay(3);
            uint8_t cmd6[] = {0x8B, 0x00, 0x50, 0x00, 0x00, 0x1f, 0x00, 0x66, 0x66 };      // SetModulationParams, br = 50000, PulseShape = non, Bw = 156,2, FreqDev = 600
            sx126xcommand(cmd6, this->rx_buffer, 9);
            uint8_t cmd8[] = {0x8C, 0x00, 0x18, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x01, 0x00}; // SetPacketParams, 
            sx126xcommand(cmd8, this->rx_buffer, 10);
            uint8_t cmd14[] = {0x96, 0x01 };                                    // SetRegulatorMode, 
            sx126xcommand(cmd14, this->rx_buffer, 2);
            uint8_t cmd17[] = {0x08, 0x03, 0xff, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00 };                 // SetDioIrqParams
            sx126xcommand(cmd17, this->rx_buffer, 9);
            uint8_t cmd5[] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };                        // GetStats
            sx126xcommand(cmd5, this->rx_buffer, 2);
            uint8_t cmd18[] = {0x80, 0x00};                       // SetStandby, STDBY_RC = 0x00 STDBY_XOSC = 0x01
            sx126xcommand(cmd18, this->rx_buffer, 2);
            uint8_t cmd7[] = {0x82, 0x00, 0x00, 0x00};                        // SetRX, no timeout
            sx126xcommand(cmd7, this->rx_buffer, 4);
            delay(3);
            sx126xcommand(cmd5, this->rx_buffer, 2);
            sx126xcommand(cmd5, this->rx_buffer, 2);
*/
        }

        void Sx126XSpiComponent::loop() {
            this->led_handler();

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
