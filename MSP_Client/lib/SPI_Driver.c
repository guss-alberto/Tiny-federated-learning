#include "SPI_Driver.h"

void SPI_Init(eUSCI_SPI_MasterConfig SPIConfig)
{
    GPIO_setAsPeripheralModuleFunctionInputPin(SPI_PORT, SPI_PINS, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsOutputPin (SPI_CS_PORT, SPI_CS_PIN|SPI_RSET_PIN);
    GPIO_setOutputHighOnPin (SPI_CS_PORT, SPI_CS_PIN|SPI_RSET_PIN);

    int i;
    for(i = 0; i < 1000000; i++);

    SPI_initMaster(EUSCI_B0_BASE, &SPIConfig);
    SPI_enableModule(EUSCI_B0_BASE);
    GPIO_setOutputLowOnPin(SPI_CS_PORT, SPI_RSET_PIN);
    for(i = 0; i < 1000000; i++);
}

void SPI_Write(void *Data, uint32_t Size)
{
    uint32_t i;
    for(i = 0; i < Size; i++)
    {
        SPI_transmitData(EUSCI_B0_BASE, ((int8_t*)Data)[i]);
    }

}

void SPI_Read(void *Data, uint32_t Size)
{
    uint32_t i;
    for(i = 0; i < Size; i++)
    {
        ((int8_t*)Data)[i] = SPI_receiveData(EUSCI_B0_BASE);
    }
}

void SPI_SendByte(uint8_t Data){
    SPI_transmitData(EUSCI_B0_BASE, Data);
}
uint8_t SPI_GetByte(){
    return SPI_receiveData(EUSCI_B0_BASE);
}
