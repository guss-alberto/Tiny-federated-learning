#include "UART_Driver.h"
#include <stdarg.h>
#include <stdio.h>
/*UARTA0 Ring Buffer Global Variables*/
volatile uint8_t UARTA0Data[UARTA0_BUFFERSIZE];
volatile uint32_t UARTA0ReadIndex;
volatile uint32_t UARTA0WriteIndex;

#define UARTA0_ADVANCE_READ_INDEX          UARTA0ReadIndex = (UARTA0ReadIndex + 1) % UARTA0_BUFFERSIZE;
#define UARTA0_ADVANCE_WRITE_INDEX         UARTA0WriteIndex = (UARTA0WriteIndex + 1) % UARTA0_BUFFERSIZE
#define UARTA0_BUFFER_EMPTY                UARTA0ReadIndex == UARTA0WriteIndex ? true : false
#define UARTA0_BUFFER_FULL                 (UARTA0WriteIndex + 1) % UARTA0_BUFFERSIZE == UARTA0ReadIndex ? true : false

void UART_Init(eUSCI_UART_ConfigV1 UARTConfig){
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A0_BASE, &UARTConfig);
    MAP_UART_enableModule(EUSCI_A0_BASE);
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
    UARTA0ReadIndex=0;
    UARTA0WriteIndex=0;
}

void UART_Write(const void *Data, uint32_t Size)
{
    uint32_t i;
    for(i = 0; i < Size; i++)
    {
        UART_transmitData(EUSCI_A0_BASE, ((int8_t*)Data)[i]);
    }
}

void UART_printf (char * format, ...)
{
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, 255, format, args);
  uint8_t i=0;
  while (buffer[i]){
      UART_transmitData(EUSCI_A0_BASE, buffer[i]);
      i++;
  }

  //do something with the error

  va_end (args);
}

void UART_Read(const void *Data, uint32_t Size)
{
    uint32_t i;
    int8_t c;

    for(i = 0; i < Size; i++)
    {
        while(UARTA0_BUFFER_EMPTY);//wait until there is enough data
        c = UARTA0Data[UARTA0ReadIndex];
        UARTA0_ADVANCE_READ_INDEX;

        ((int8_t*)Data)[i] = c;
    }
}

//non blocking, returns the number of bytes read
uint32_t UART_Read_nb(const void *Data, uint32_t Size)
{
    uint32_t i;
    int8_t c;

    for(i = 0; i < Size; i++)
    {
        if (UARTA0_BUFFER_EMPTY){
            return i;
        }
        c = UARTA0Data[UARTA0ReadIndex];
        UARTA0_ADVANCE_READ_INDEX;

        ((int8_t*)Data)[i] = c;
    }
    return i;
}

void EUSCIA0_IRQHandler(void)
{
    uint8_t c;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        c = UART_receiveData(EUSCI_A0_BASE);
        UARTA0Data[UARTA0WriteIndex] = c;
        UARTA0_ADVANCE_WRITE_INDEX;
    }
}

