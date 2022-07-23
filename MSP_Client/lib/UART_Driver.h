#ifndef HARDWARE_UART_DRIVER_H_
#define HARDWARE_UART_DRIVER_H_

#include <ti/devices/msp432p4xx/driverlib/rom.h>
#include <ti/devices/msp432p4xx/driverlib/rom_map.h>
#include <ti/devices/msp432p4xx/driverlib/interrupt.h>
#include <ti/devices/msp432p4xx/driverlib/uart.h>
#include <ti/devices/msp432p4xx/driverlib/gpio.h>

#define UARTA0_BUFFERSIZE 128

void UART_Init(uint32_t UART, eUSCI_UART_ConfigV1 UARTConfig);
void UART_Write(const void *Data, uint32_t Size);
void UART_Read(const void *Data, uint32_t Size);
uint32_t UART_Read_nb(const void *Data, uint32_t Size); //non blocking, returns the number of bytes read


#endif /* HARDWARE_UART_DRIVER_H_ */
