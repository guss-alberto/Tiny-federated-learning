#ifndef LIB_LORA_LORA_H_
#define LIB_LORA_LORA_H_

#include "../../includes.h"


#define SPI_CS_PORT    GPIO_PORT_P5
#define SPI_CS_PIN    GPIO_PIN0
#define SPI_RSET_PIN   GPIO_PIN1

#define SPI_PORT GPIO_PORT_P1
#define SPI_MISO_PIN  GPIO_PIN7
#define SPI_MOSI_PIN  GPIO_PIN6
#define SPI_CLOCK_PIN GPIO_PIN5

void selectModule(uint8_t pin);
uint8_t readRegister(uint8_t address);

void LoRa_Init();
uint8_t LoRa_Begin (long frequency);

void end();

int beginPacket(int implicitHeade);
int endPacket(bool async);

int LoRa_parsePacket(int size);
int LoRa_packetRssi();
float packetSnr();
long packetFrequencyError();

int rssi();

// from Print
size_t LoRa_write(const uint8_t *buffer, size_t size);

// from Stream
int LoRa_available();
int LoRa_read();
int peek();

void idle();
void sleep();

void setTxPower(int level, int outputPin);
void setFrequency(uint64_t frequency);
void setSpreadingFactor(int sf);
void setSignalBandwidth(long sbw);
void setCodingRate4(int denominator);
void setPreambleLength(long length);
void setSyncWord(int sw);
void enableCrc();
void disableCrc();
void enableInvertIQ();
void disableInvertIQ();

void setOCP(uint8_t mA); // Over Current Protection control

void setGain(uint8_t gain); // Set LNA gain



// registers
#define REG_FIFO                 0x00
#define REG_OP_MODE              0x01
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08
#define REG_PA_CONFIG            0x09
#define REG_OCP                  0x0b
#define REG_LNA                  0x0c
#define REG_FIFO_ADDR_PTR        0x0d
#define REG_FIFO_TX_BASE_ADDR    0x0e
#define REG_FIFO_RX_BASE_ADDR    0x0f
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_IRQ_FLAGS            0x12
#define REG_RX_NB_BYTES          0x13
#define REG_PKT_SNR_VALUE        0x19
#define REG_PKT_RSSI_VALUE       0x1a
#define REG_RSSI_VALUE           0x1b
#define REG_MODEM_CONFIG_1       0x1d
#define REG_MODEM_CONFIG_2       0x1e
#define REG_PREAMBLE_MSB         0x20
#define REG_PREAMBLE_LSB         0x21
#define REG_PAYLOAD_LENGTH       0x22
#define REG_MODEM_CONFIG_3       0x26
#define REG_FREQ_ERROR_MSB       0x28
#define REG_FREQ_ERROR_MID       0x29
#define REG_FREQ_ERROR_LSB       0x2a
#define REG_RSSI_WIDEBAND        0x2c
#define REG_DETECTION_OPTIMIZE   0x31
#define REG_INVERTIQ             0x33
#define REG_DETECTION_THRESHOLD  0x37
#define REG_SYNC_WORD            0x39
#define REG_INVERTIQ2            0x3b
#define REG_DIO_MAPPING_1        0x40
#define REG_VERSION              0x42
#define REG_PA_DAC               0x4d

// modes
#define MODE_LONG_RANGE_MODE     0x80
#define MODE_SLEEP               0x00
#define MODE_STDBY               0x01
#define MODE_TX                  0x03
#define MODE_RX_CONTINUOUS       0x05
#define MODE_RX_SINGLE           0x06

// PA config
#define PA_BOOST                 0x80

// IRQ masks
#define IRQ_TX_DONE_MASK           0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20
#define IRQ_RX_DONE_MASK           0x40

#define RF_MID_BAND_THRESHOLD    525E6
#define RSSI_OFFSET_HF_PORT      157
#define RSSI_OFFSET_LF_PORT      164

#define MAX_PKT_LENGTH           255

#define PA_OUTPUT_RFO_PIN          0
#define PA_OUTPUT_PA_BOOST_PIN     1

#endif /* LIB_LORA_LORA_H_ */
