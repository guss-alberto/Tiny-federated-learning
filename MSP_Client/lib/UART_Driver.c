#include "UART_Driver.h"

/*UARTA0 Ring Buffer Global Variables*/
volatile uint8_t UARTA0Data[UARTA0_BUFFERSIZE];
volatile uint32_t UARTA0ReadIndex;
volatile uint32_t UARTA0WriteIndex;

#define UARTA0_ADVANCE_READ_INDEX          UARTA0ReadIndex = (UARTA0ReadIndex + 1) % UARTA0_BUFFERSIZE;
#define UARTA0_ADVANCE_WRITE_INDEX         UARTA0WriteIndex = (UARTA0WriteIndex + 1) % UARTA0_BUFFERSIZE
#define UARTA0_BUFFER_EMPTY                UARTA0ReadIndex == UARTA0WriteIndex ? true : false
#define UARTA0_BUFFER_FULL                 (UARTA0WriteIndex + 1) % UARTA0_BUFFERSIZE == UARTA0ReadIndex ? true : false

void UART_Init(uint32_t UART, eUSCI_UART_ConfigV1 UARTConfig){
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(UART, &UARTConfig);
    MAP_UART_enableModule(UART);
    MAP_UART_enableInterrupt(UART, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
}

void UART_Write(uint8_t *Data, uint32_t Size)
{
    uint32_t i;
    for(i = 0; i < Size; i++)
    {
        MAP_UART_transmitData(EUSCI_A0_BASE, Data[i]);
    }
}

void UART_Read(uint8_t *Data, uint32_t Size)
{
    uint32_t i;
    int8_t c;

    for(i = 0; i < Size; i++)
    {
        while(UARTA0_BUFFER_EMPTY);//wait until there is enough data
        c = UARTA0Data[UARTA0ReadIndex];
        UARTA0_ADVANCE_READ_INDEX;

        Data[i] = c;
    }
}

//non blocking, returns the number of bytes read
uint32_t UART_Read_nb(uint8_t *Data, uint32_t Size)
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

        Data[i] = c;
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
        c = MAP_UART_receiveData(EUSCI_A0_BASE);
        UARTA0Data[UARTA0WriteIndex] = c;
        UARTA0_ADVANCE_WRITE_INDEX;

        /*Transmit data only if it made it to the buffer*/
        MAP_UART_transmitData(EUSCI_A0_BASE, c);
    }
}

