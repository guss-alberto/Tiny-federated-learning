#include "LoRa.h"

uint64_t _frequency;
int _packetIndex;
int _implicitHeaderMode;
void (*_onReceive)(int);
void (*_onTxDone)();

void explicitHeaderMode();
void implicitHeaderMode();

void handleDio0Rise();
bool isTransmitting();

int getSpreadingFactor();
long getSignalBandwidth();

void setLdoFlag();

uint8_t readRegister(uint8_t address);
void writeRegister(uint8_t address, uint8_t value);

void bitWrite(uint8_t *x, uint8_t n, uint8_t value)
{
  if (value)
    *x |= (1 << n);
  else
    *x &= ~(1 << n);
}

uint8_t bitRead(uint8_t *x, uint8_t n)
{
  return (*x & (1 << n)) ? 1 : 0;
}

int beginPacket(int implicitHeader)
{
  if (isTransmitting())
  {
    return 0;
  }

  // put in standby mode
  idle();

  if (implicitHeader)
  {
    implicitHeaderMode();
  }
  else
  {
    explicitHeaderMode();
  }

  // reset FIFO address and paload length
  writeRegister(REG_FIFO_ADDR_PTR, 0);
  writeRegister(REG_PAYLOAD_LENGTH, 0);

  return 1;
}

int endPacket(bool async)
{

  if ((async) && (_onTxDone))
    writeRegister(REG_DIO_MAPPING_1, 0x40); // DIO0 => TXDONE

  // put in TX mode
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);

  if (!async)
  {
    // wait for TX done
    while ((readRegister(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0)
      ;
    // clear IRQ's
    writeRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
  }

  return 1;
}

bool isTransmitting()
{
  if ((readRegister(REG_OP_MODE) & MODE_TX) == MODE_TX)
  {
    return true;
  }

  if (readRegister(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK)
  {
    // clear IRQ's
    writeRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
  }

  return false;
}

int LoRa_parsePacket(int size)
{
  int packetLength = 0;
  int irqFlags = readRegister(REG_IRQ_FLAGS);

  if (size > 0)
  {
    implicitHeaderMode();

    writeRegister(REG_PAYLOAD_LENGTH, size & 0xff);
  }
  else
  {
    explicitHeaderMode();
  }

  // clear IRQ's
  writeRegister(REG_IRQ_FLAGS, irqFlags);

  if ((irqFlags & IRQ_RX_DONE_MASK) && (irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0)
  {
    // received a packet
    _packetIndex = 0;

    // read packet length
    if (_implicitHeaderMode)
    {
      packetLength = readRegister(REG_PAYLOAD_LENGTH);
    }
    else
    {
      packetLength = readRegister(REG_RX_NB_BYTES);
    }

    // set FIFO address to current RX address
    writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));

    // put in standby mode
    idle();
  }
  else if (readRegister(REG_OP_MODE) != (MODE_LONG_RANGE_MODE | MODE_RX_SINGLE))
  {
    // not currently in RX mode

    // reset FIFO address
    writeRegister(REG_FIFO_ADDR_PTR, 0);

    // put in single RX mode
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_SINGLE);
  }

  return packetLength;
}

int LoRa_packetRssi()
{
  return (readRegister(REG_PKT_RSSI_VALUE) - (_frequency < RF_MID_BAND_THRESHOLD ? RSSI_OFFSET_LF_PORT : RSSI_OFFSET_HF_PORT));
}

float packetSnr()
{
  return ((int8_t)readRegister(REG_PKT_SNR_VALUE)) * 0.25;
}

long packetFrequencyError()
{
  int32_t freqError = 0;
  freqError = (int32_t)(readRegister(REG_FREQ_ERROR_MSB) & 0b111);
  freqError <<= 8L;
  freqError += (int32_t)(readRegister(REG_FREQ_ERROR_MID));
  freqError <<= 8L;
  freqError += (int32_t)(readRegister(REG_FREQ_ERROR_LSB));

  if (readRegister(REG_FREQ_ERROR_MSB) & 0b1000)
  {                      // Sign bit is on
    freqError -= 524288; // 0b1000'0000'0000'0000'0000
  }

  const float fXtal = 32E6;                                                                              // FXOSC: crystal oscillator (XTAL) frequency (2.5. Chip Specification, p. 14)
  const float fError = ((((float)freqError) * (1L << 24)) / fXtal) * (getSignalBandwidth() / 500000.0f); // p. 37

  return (long)(fError);
}

int rssi()
{
  return (readRegister(REG_RSSI_VALUE) - (_frequency < RF_MID_BAND_THRESHOLD ? RSSI_OFFSET_LF_PORT : RSSI_OFFSET_HF_PORT));
}

size_t LoRa_write(const uint8_t *buffer, size_t size)
{
  int currentLength = readRegister(REG_PAYLOAD_LENGTH);

  // check size
  if ((currentLength + size) > MAX_PKT_LENGTH)
  {
    size = MAX_PKT_LENGTH - currentLength;
  }

  // write data
  uint32_t i;
  for (i = 0; i < size; i++)
  {
    writeRegister(REG_FIFO, buffer[i]);
  }

  // update length
  writeRegister(REG_PAYLOAD_LENGTH, currentLength + size);

  return size;
}

int LoRa_available()
{
  return (readRegister(REG_RX_NB_BYTES) - _packetIndex);
}

int LoRa_read()
{
  if (!LoRa_available())
  {
    return -1;
  }

  _packetIndex++;

  return readRegister(REG_FIFO);
}

int peek()
{
  if (!LoRa_available())
  {
    return -1;
  }

  // store current FIFO address
  int currentAddress = readRegister(REG_FIFO_ADDR_PTR);

  // read
  uint8_t b = readRegister(REG_FIFO);

  // restore FIFO address
  writeRegister(REG_FIFO_ADDR_PTR, currentAddress);

  return b;
}

void idle()
{
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

void sleep()
{
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

void setTxPower(int level, int outputPin)
{
  if (PA_OUTPUT_RFO_PIN == outputPin)
  {
    // RFO
    if (level < 0)
    {
      level = 0;
    }
    else if (level > 14)
    {
      level = 14;
    }

    writeRegister(REG_PA_CONFIG, 0x70 | level);
  }
  else
  {
    // PA BOOST
    if (level > 17)
    {
      if (level > 20)
      {
        level = 20;
      }

      // subtract 3 from level, so 18 - 20 maps to 15 - 17
      level -= 3;

      // High Power +20 dBm Operation (Semtech SX1276/77/78/79 5.4.3.)
      writeRegister(REG_PA_DAC, 0x87);
      setOCP(140);
    }
    else
    {
      if (level < 2)
      {
        level = 2;
      }
      // Default value PA_HF/LF or +17dBm
      writeRegister(REG_PA_DAC, 0x84);
      setOCP(100);
    }

    writeRegister(REG_PA_CONFIG, PA_BOOST | (level - 2));
  }
}

void setFrequency(uint64_t frequency)
{
  _frequency = frequency;

  uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

  writeRegister(REG_FRF_MSB, (uint8_t)(frf >> 16));
  writeRegister(REG_FRF_MID, (uint8_t)(frf >> 8));
  writeRegister(REG_FRF_LSB, (uint8_t)(frf >> 0));
}

int getSpreadingFactor()
{
  return readRegister(REG_MODEM_CONFIG_2) >> 4;
}

void setSpreadingFactor(int sf)
{
  if (sf < 6)
  {
    sf = 6;
  }
  else if (sf > 12)
  {
    sf = 12;
  }

  if (sf == 6)
  {
    writeRegister(REG_DETECTION_OPTIMIZE, 0xc5);
    writeRegister(REG_DETECTION_THRESHOLD, 0x0c);
  }
  else
  {
    writeRegister(REG_DETECTION_OPTIMIZE, 0xc3);
    writeRegister(REG_DETECTION_THRESHOLD, 0x0a);
  }

  writeRegister(REG_MODEM_CONFIG_2, (readRegister(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));
  setLdoFlag();
}

long getSignalBandwidth()
{
  uint8_t bw = (readRegister(REG_MODEM_CONFIG_1) >> 4);

  switch (bw)
  {
  case 0:
    return 7.8E3;
  case 1:
    return 10.4E3;
  case 2:
    return 15.6E3;
  case 3:
    return 20.8E3;
  case 4:
    return 31.25E3;
  case 5:
    return 41.7E3;
  case 6:
    return 62.5E3;
  case 7:
    return 125E3;
  case 8:
    return 250E3;
  case 9:
    return 500E3;
  }

  return -1;
}

void setSignalBandwidth(long sbw)
{
  int bw;

  if (sbw <= 7.8E3)
  {
    bw = 0;
  }
  else if (sbw <= 10.4E3)
  {
    bw = 1;
  }
  else if (sbw <= 15.6E3)
  {
    bw = 2;
  }
  else if (sbw <= 20.8E3)
  {
    bw = 3;
  }
  else if (sbw <= 31.25E3)
  {
    bw = 4;
  }
  else if (sbw <= 41.7E3)
  {
    bw = 5;
  }
  else if (sbw <= 62.5E3)
  {
    bw = 6;
  }
  else if (sbw <= 125E3)
  {
    bw = 7;
  }
  else if (sbw <= 250E3)
  {
    bw = 8;
  }
  else /*if (sbw <= 250E3)*/
  {
    bw = 9;
  }

  writeRegister(REG_MODEM_CONFIG_1, (readRegister(REG_MODEM_CONFIG_1) & 0x0f) | (bw << 4));
  setLdoFlag();
}

void setLdoFlag()
{
  // Section 4.1.1.5
  long symbolDuration = 1000 / (getSignalBandwidth() / (1L << getSpreadingFactor()));

  // Section 4.1.1.6
  bool ldoOn = symbolDuration > 16;

  uint8_t config3 = readRegister(REG_MODEM_CONFIG_3);
  bitWrite(&config3, 3, ldoOn);
  writeRegister(REG_MODEM_CONFIG_3, config3);
}

void setCodingRate4(int denominator)
{
  if (denominator < 5)
  {
    denominator = 5;
  }
  else if (denominator > 8)
  {
    denominator = 8;
  }

  int cr = denominator - 4;

  writeRegister(REG_MODEM_CONFIG_1, (readRegister(REG_MODEM_CONFIG_1) & 0xf1) | (cr << 1));
}

void setPreambleLength(long length)
{
  writeRegister(REG_PREAMBLE_MSB, (uint8_t)(length >> 8));
  writeRegister(REG_PREAMBLE_LSB, (uint8_t)(length >> 0));
}

void setSyncWord(int sw)
{
  writeRegister(REG_SYNC_WORD, sw);
}

void enableCrc()
{
  writeRegister(REG_MODEM_CONFIG_2, readRegister(REG_MODEM_CONFIG_2) | 0x04);
}

void disableCrc()
{
  writeRegister(REG_MODEM_CONFIG_2, readRegister(REG_MODEM_CONFIG_2) & 0xfb);
}

void enableInvertIQ()
{
  writeRegister(REG_INVERTIQ, 0x66);
  writeRegister(REG_INVERTIQ2, 0x19);
}

void disableInvertIQ()
{
  writeRegister(REG_INVERTIQ, 0x27);
  writeRegister(REG_INVERTIQ2, 0x1d);
}

void setOCP(uint8_t mA)
{
  uint8_t ocpTrim = 27;

  if (mA <= 120)
  {
    ocpTrim = (mA - 45) / 5;
  }
  else if (mA <= 240)
  {
    ocpTrim = (mA + 30) / 10;
  }

  writeRegister(REG_OCP, 0x20 | (0x1F & ocpTrim));
}

void setGain(uint8_t gain)
{
  // check allowed range
  if (gain > 6)
  {
    gain = 6;
  }

  // set to standby
  idle();

  // set gain
  if (gain == 0)
  {
    // if gain = 0, enable AGC
    writeRegister(REG_MODEM_CONFIG_3, 0x04);
  }
  else
  {
    // disable AGC
    writeRegister(REG_MODEM_CONFIG_3, 0x00);

    // clear Gain and set LNA boost
    writeRegister(REG_LNA, 0x03);

    // set gain
    writeRegister(REG_LNA, readRegister(REG_LNA) | (gain << 5));
  }
}

void explicitHeaderMode()
{
  _implicitHeaderMode = 0;

  writeRegister(REG_MODEM_CONFIG_1, readRegister(REG_MODEM_CONFIG_1) & 0xfe);
}

void implicitHeaderMode()
{
  _implicitHeaderMode = 1;

  writeRegister(REG_MODEM_CONFIG_1, readRegister(REG_MODEM_CONFIG_1) | 0x01);
}

#define BIT_BANG

void LoRa_Init()
{
#ifdef BIT_BANG
  GPIO_setOutputLowOnPin(SPI_PORT, SPI_CLOCK_PIN);             // disable clock
  GPIO_setAsOutputPin(SPI_PORT, SPI_MOSI_PIN | SPI_CLOCK_PIN); // set MISO and clock
  GPIO_setAsInputPin(SPI_PORT, SPI_MISO_PIN);
#endif

  GPIO_setAsOutputPin(SPI_CS_PORT, SPI_CS_PIN | SPI_RSET_PIN);
  GPIO_setOutputHighOnPin(SPI_CS_PORT, SPI_CS_PIN | SPI_RSET_PIN);

#ifndef BIT_BANG
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(SPI_PORT, SPI_MOSI_PIN | SPI_CLOCK_PIN | SPI_MISO_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
  eUSCI_SPI_MasterConfig config =
      {
          EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
          48000000,
          10000,
          EUSCI_B_SPI_MSB_FIRST,
          EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
          EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,
          EUSCI_B_SPI_3PIN};

  SPI_initMaster(EUSCI_B0_BASE, &config);
  SPI_enableModule(EUSCI_B0_BASE);
#endif

  int i;
  for (i = 0; i < 10000; i++)
    ;
  GPIO_setOutputLowOnPin(SPI_CS_PORT, SPI_RSET_PIN); // wait for board to reset
  for (i = 0; i < 10000; i++)
    ;
}

#define BANDWIDTH 0b00
/*Signal bandwidth:
    00 -> 125 kHz
    01 -> 250 kHz
    10 -> 500 kHz
*/
#define ERROR_CODING_RATE 0b001
/*Error coding rate
    001 -> 4/5
    010 -> 4/6
    011 -> 4/7
    100 -> 4/8
*/
#define SPREADING_FACTOR 7
/*SF rate (expressed as a base-2 logarithm)
    6  -> 64   chips / symbol
    7  -> 128  chips / symbol
    8  -> 256  chips / symbol
    9  -> 512  chips / symbol
    10 -> 1024 chips / symbol
    11 -> 2048 chips / symbol
    12 -> 4096 chips / symbol
*/
#define AUTOMATIC_GAIN_CONTROL true

#define LNA_GAIN 0b001
/*LNA gain setting:
    000 -> reserved
    001 -> G1 = highest gain
    010 -> G2 = highest gain – 6 dB
    011 -> G3 = highest gain – 12 dB
    100 -> G4 = highest gain – 24 dB
    101 -> G5 = highest gain – 36 dB
    110 -> G6 = highest gain – 48 dB
    111 -> reserved
*/
#define LNA_BOOST 0b00
/* Improves the system Noise Figure at the expense of Rx current
consumption:
00 -> Default setting, meeting the specification
11 -> Improved sensitivity
 */
uint8_t LoRa_Begin(long frequency)
{
  uint8_t version = readRegister(REG_VERSION);
  if (version != 0x22)
  {
    return 0;
  }

  // put in sleep mode
  sleep();

  // set frequency
  // setFrequency(frequency);

  // set base addresses
  writeRegister(REG_FIFO_TX_BASE_ADDR, 0);
  writeRegister(REG_FIFO_RX_BASE_ADDR, 0);

  // set bandwidth, error coding rate
  writeRegister(REG_MODEM_CONFIG_1, (BANDWIDTH << 6) | (ERROR_CODING_RATE) | 0b000);
  // set
  writeRegister(REG_MODEM_CONFIG_2, (SPREADING_FACTOR << 4) | (AUTOMATIC_GAIN_CONTROL << 2) | 0b10);

  // set LNA boost
  writeRegister(REG_LNA, (LNA_GAIN << 5) | LNA_BOOST);

  // set output power to 17 dBm
  setTxPower(20, 0);

  // put in standby mode
  idle();

  return 1;
}

#ifndef BIT_BANG
uint8_t readRegister(uint8_t address)
{
  P5->OUT &= ~SPI_CS_PIN;
  address &= 0x7F; // set first bit to 0 for read
  MAP_SPI_transmitData(EUSCI_B0_BASE, address);
  address = MAP_SPI_receiveData(EUSCI_B0_BASE);
  P5->OUT |= SPI_CS_PIN;
  return address;
}
void writeRegister(uint8_t address, uint8_t value)
{
  P5->OUT &= ~SPI_CS_PIN;
  address &= 0x7F; // set first bit to 0 for read
  MAP_SPI_transmitData(EUSCI_B0_BASE, address);
  MAP_SPI_transmitData(EUSCI_B0_BASE, value);
  P5->OUT |= SPI_CS_PIN;
}
#else

inline void sendByteSPI(uint8_t byte)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if (byte & (128 >> i))
      // GPIO_setOutputHighOnPin (SPI_PORT, SPI_MOSI_PIN);
      P1->OUT |= SPI_MOSI_PIN;
    else
      // GPIO_setOutputLowOnPin (SPI_PORT, SPI_MOSI_PIN);
      P1->OUT &= ~SPI_MOSI_PIN;
    // GPIO_setOutputHighOnPin (SPI_PORT, SPI_CLOCK_PIN);  //pulse clock
    // GPIO_setOutputLowOnPin (SPI_PORT, SPI_CLOCK_PIN);
    P1->OUT |= SPI_CLOCK_PIN;
    P1->OUT &= ~SPI_CLOCK_PIN;
  }
}

uint8_t readRegister(uint8_t address)
{
  uint8_t i, r = 0;
  address &= 0x7F; // set first bit to 0 for read

  // enable cs
  // GPIO_setOutputLowOnPin (SPI_CS_PORT, SPI_CS_PIN);
  P5->OUT &= ~SPI_CS_PIN;

  sendByteSPI(address);

  for (i = 0; i < 8; i++)
  { // read
    // GPIO_setOutputHighOnPin (SPI_PORT, SPI_CLOCK_PIN);
    P1->OUT |= SPI_CLOCK_PIN;
    // if (GPIO_getInputPinValue(SPI_PORT, SPI_MISO_PIN))
    if (P1->IN & SPI_MISO_PIN)
      r |= (128 >> i); // set the correct bit
    // GPIO_setOutputLowOnPin (SPI_PORT, SPI_CLOCK_PIN);
    P1->OUT &= ~SPI_CLOCK_PIN;
  }

  // end transaction with cs high
  // GPIO_setOutputHighOnPin (SPI_CS_PORT, SPI_CS_PIN);
  P5->OUT |= SPI_CS_PIN;
  return r;
}

void writeRegister(uint8_t address, uint8_t value)
{
  address |= 0x80; // set first bit to 1 for write

  // enable cs
  // GPIO_setOutputLowOnPin (SPI_CS_PORT, SPI_CS_PIN);
  P5->OUT &= ~SPI_CS_PIN;

  sendByteSPI(address);
  sendByteSPI(value);

  // end transaction with cs high
  // GPIO_setOutputHighOnPin (SPI_CS_PORT, SPI_CS_PIN);
  P5->OUT |= SPI_CS_PIN;
}

#endif
