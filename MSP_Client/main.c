#include "includes.h"
#include "microphone.h"
#include "fft.h"
#include "ml.h"
#include "lib/test.h"
#include "stdio.h"

void init();
uint8_t buttons();
void simple_train(uint8_t class);

#define REC_DELAY 200000

uint8_t mode = MODE_RECORD;

union
{ // use union to save memory
    int16_t rec[NUM_SAMPLES];
    struct
    {
        float input[NODES_L0];
        float out[NODES_L2];
        float target[NODES_L2];
        float error;
    } ml;
} myData;

int main(void)
{
    init();

#if defined RECEIVERMODE
    uint8_t a;
    UART_printf("test start\n");
    while (1)
    {
        int packetSize = LoRa_parsePacket(0);
        if (packetSize)
        {
            UART_printf("packetSize = %d\n", packetSize);
            // read packet
            while (LoRa_available())
            {
                a = LoRa_read();
                UART_Write(&a, 1);
            }
            // print RSSI of packet
            // UART_printf("With RSSI %d\n", LoRa_packetRssi());
        }

        if (UART_Read_nb(&a, 1))
        {
            uint8_t buffer[256];
            UART_Read(buffer, a);

            beginPacket(false);
            LoRa_write(buffer, a);
            endPacket(false);
        }
    }
#elif defined TEST_SENDER
    UART_printf("test start\n");

    beginPacket(false);
    LoRa_write("Hello, world!\n", 14);
    endPacket(false);
    UART_printf("Message Sent!\n");

    UART_printf("Test message Sent!\n");
    uint32_t time = millis();

    memset(weights_L1, 'a', sizeof(weights_L1));
    memset(weights_L2, 'b', sizeof(weights_L2));

    sendModel();
    UART_printf("Long message sent in %d ms\n", millis() - time);

#else
    init_mfcc();
    _micInit();
    ml_init();
    uint8_t a;
    uint32_t i;

    UART_printf("test start\n");

    // send packet
    selectModule(SPI_CS1_PIN);
    /*beginPacket(false);
    write("Hello, world!\n",14);
    endPacket(false);
    UART_printf("Message Sent!\n");


    while(1){
        int packetSize = parsePacket(0);
        if (packetSize)  {
            UART_printf("packetSize = %d\n",packetSize);
                // read packet
            while (available()) {
              a = read();
              UART_Write(&a,1);
            }
        // print RSSI of packet
        UART_printf("With RSSI %d\n", packetRssi());
        }
    }*/
    Graphics_drawString(&ctx, (int8_t *)modeStr[(mode)-1], 20, 20, 10, true);
    srand(RANDOM_SEED);
    while (1)
    {
        int packetSize = parsePacket(0);
        // if message available on LoRa
        if (packetSize)
        {
            a = LoRa_read(); // read device ID
            if (a == DEVICE_ID)
            {
                a = LoRa_read(); // read packet type
                switch (a)
                {
                case FL_READY:
                    LoRa_write(&a, 1);                           // send message back to indicate ready status
                    LoRa_write(&num_epochs, sizeof(num_epochs)); // send number of epochs
                    sendModel();                                 // send NN weights
                    num_epochs = 0;                              // reset number of epochs to zero
                    break;
                case RECEIVE_TRAINING:
                    a = REQUEST_TRAINING_DATA;
                    beginPacket(false);
                    LoRa_write(&a, 1);
                    endPacket(false); // send request for training data
                    // Graphics_drawString(&ctx, "using serial data", 20, 20, 20, true);

                    LoRa_getLarge(myData.ml.input, sizeof(myData.ml.input)); // Wait for data input

                    while (packetSize = parsePacket(0);)
                        a = LoRa_read(); // get output class

                    simple_train(a);
                    beginPacket(false);
                    LoRa_Write(&myData.ml.error, sizeof(float));
                    endPacket(false);
                    break;
                }
            }
            else if (a == BROADCAST)
            {
                a = LoRa_read(); // read device ID
                switch (a)
                {
                case NEW_MODEL:
                    getModel();
                    break;
                }
            }
        }
        else
        {

            switch (mode)
            {
            case MODE_LEARN:
                a = buttons();
                if (!a)
                    break; // if no buttons are pushed exit and continue the loop
                for (i = 0; i < REC_DELAY; i++)
                    ;
                Graphics_drawString(&ctx, "sampling.....", 20, 20, 20, true);
                micSample(myData.rec);
                Graphics_drawString(&ctx, "processing...", 20, 10, 20, true);
                feature_extraction(myData.rec, myData.ml.input);

                simple_train(a);
                break;
            case MODE_EVAL:
                a = buttons();
                if (!a)
                    break; // if no buttons are pushed exit and continue the loop
                for (i = 0; i < REC_DELAY; i++)
                    ;
                Graphics_drawString(&ctx, "sampling.....", 20, 10, 20, true);
                micSample(myData.rec);
                Graphics_drawString(&ctx, "processing...", 20, 10, 20, true);
                feature_extraction(myData.rec, myData.ml.input);

                eval(myData.ml.input, myData.ml.out, myData.ml.out);
                char str[30];
                sprintf(str, "%.2f | %.2f | %.2f", myData.ml.out[0], myData.ml.out[1], myData.ml.out[2]);

                Graphics_drawString(&ctx, "DONE!........", 20, 10, 20, true);
                Graphics_drawString(&ctx, (int8_t *)str, 30, 10, 30, true);
                break;
            case MODE_RECORD:
                a = buttons();
                if (a)
                {
                    for (i = 0; i < REC_DELAY; i++)
                        ;
                    Graphics_drawString(&ctx, "sampling.....", 20, 10, 20, true);
                    micSample(myData.rec);
                    Graphics_drawString(&ctx, "processing...", 20, 10, 20, true);
                    feature_extraction(myData.rec, myData.ml.input);
                    const char tmp = DATA_READY;
                    UART_Write(&tmp, 1); // send ready message

                    UART_Write(myData.ml.input, sizeof(myData.ml.input));
                    UART_Write(&a, 1); // send class
                    Graphics_drawString(&ctx, "DONE!........", 20, 10, 20, true);
                }
                break;
            }
        }
    }
#endif
}

void simple_train(uint8_t class)
{
    myData.ml.target[0] = 0;
    myData.ml.target[1] = 0;
    myData.ml.target[2] = 0;
    myData.ml.target[class - 1] = 1.0; // button 1 -> {1,0,0};  button 2 -> {0,1,0};  button 3 -> {0,0,1}

    myData.ml.error = learn(myData.ml.input, myData.ml.out, myData.ml.target); // train

    char str[30];
    sprintf(str, "%.2f | %.2f | %.2f", myData.ml.out[0], myData.ml.out[1], myData.ml.out[2]);

    Graphics_drawString(&ctx, "DONE!........", 20, 10, 20, true);
    Graphics_drawString(&ctx, (int8_t *)str, 30, 10, 30, true);
    sprintf(str, "ERR: %.2f", myData.ml.error);
    Graphics_drawString(&ctx, (int8_t *)str, 30, 10, 40, true);

    num_epochs++;
}

uint8_t buttons()
{
    uint8_t a = 0;
    if (!GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5) && !a)
    { // A
        a = 1;
        Graphics_drawString(&ctx, "release btn", 20, 10, 20, true);
        while (!GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5))
            ;
    }
    if (!GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1) && !a)
    { // B
        a = 2;
        Graphics_drawString(&ctx, "release btn", 20, 10, 20, true);
        while (!GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN1))
            ;
    }
    if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN1) && !a)
    { // J
        a = 3;
        Graphics_drawString(&ctx, "release btn", 20, 10, 20, true);
        while (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN1))
            ;
    }

    if (!GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) && !a)
    { // A
        a = 1;
        Graphics_drawString(&ctx, "release btn", 20, 10, 20, true);
        while (!GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1))
            ;
    }
    return a;
}

const eUSCI_UART_ConfigV1 UART0Config =
    {
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,
        13,
        0,
        27,
        EUSCI_A_UART_NO_PARITY,
        EUSCI_A_UART_LSB_FIRST,
        EUSCI_A_UART_ONE_STOP_BIT,
        EUSCI_A_UART_MODE,
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,
        EUSCI_A_UART_8_BIT_LEN};

void init()
{
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    // left button on msp board
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    // BUTTON B
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);
    // BUTTON A
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN5);
    // BUTTON J
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN1);

    /* Set the core voltage level to VCORE1 */
    PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    init_timer();
    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&ctx, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);

    Graphics_setForegroundColor(&ctx, GRAPHICS_COLOR_BLACK);
    Graphics_setBackgroundColor(&ctx, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&ctx, &g_sFontFixed6x8);
    Graphics_clearDisplay(&ctx);

    UART_Init(UART0Config);
    LoRa_Init(); // initialize pins and reset lora boards
    if (!LoRa_Begin(915E6))
    {
        UART_printf("Board failed to initialize\n");
    }
}
