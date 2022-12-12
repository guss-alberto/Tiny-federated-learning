#ifndef HARDWARE_SPI_DRIVER_H_
#define HARDWARE_SPI_DRIVER_H_

#include "../includes.h"

#include <ti/devices/msp432p4xx/driverlib/rom.h>
#include <ti/devices/msp432p4xx/driverlib/rom_map.h>
#include <ti/devices/msp432p4xx/driverlib/spi.h>
#include <ti/devices/msp432p4xx/driverlib/gpio.h>

#define SPI_PORT GPIO_PORT_P1
#define SPI_CS_PORT  GPIO_PORT_P5
#define SPI_CS_PIN  GPIO_PIN0
#define SPI_RSET_PIN  GPIO_PIN1
#define SPI_PINS GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7

void SPI_Init(eUSCI_SPI_MasterConfig SPIConfig);
void SPI_Write(void *Data, uint32_t Size);
void SPI_Read(void *Data, uint32_t Size);

void SPI_SendByte(uint8_t Data);
uint8_t SPI_GetByte();


#endif /* HARDWARE_SPI_DRIVER_H_ */
