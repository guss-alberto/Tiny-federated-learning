#include "LoRa.h"



uint8_t readRegister(uint8_t address){
    //SPI_SendByte(address & 0x7f);
    int i, j;
    GPIO_setOutputLowOnPin (SPI_CS_PORT, SPI_CS_PIN);
    for(j = 0; j < 10000; j++);

    GPIO_setOutputLowOnPin (SPI_PORT, GPIO_PIN5);
    for(j = 0; j < 10000; j++);


    for (i=0; i<8; i++){


        for(j = 0; j < 10000; j++);
        if (address&(128>>i))
            GPIO_setOutputHighOnPin (SPI_PORT, GPIO_PIN6);
        else
            GPIO_setOutputLowOnPin (SPI_PORT, GPIO_PIN6);

        for(j = 0; j < 10000; j++);
        GPIO_setOutputHighOnPin (SPI_PORT, GPIO_PIN5);
        for(j = 0; j < 10000; j++);
        GPIO_setOutputLowOnPin (SPI_PORT, GPIO_PIN5);

    }

    uint8_t r = 0; /*SPI_GetByte();
    r = SPI_GetByte();*/
    for(j = 0; j < 10000; j++);

    for (i=0; i<8; i++){
        GPIO_setOutputHighOnPin (SPI_PORT, GPIO_PIN5);

        for(j = 0; j < 10000; j++);
        if (GPIO_getInputPinValue(SPI_PORT, GPIO_PIN7)){
            r |= (128>>i);
        }
        GPIO_setOutputLowOnPin (SPI_PORT, GPIO_PIN5);
        for(j = 0; j < 10000; j++);
    }
    for(j = 0; j < 10000; j++);

    GPIO_setOutputHighOnPin (SPI_CS_PORT, SPI_CS_PIN);

    return r;
}

void writeRegister(uint8_t address, uint8_t value) {

    SPI_SendByte(address | 0x80);
    SPI_SendByte(value);

}

