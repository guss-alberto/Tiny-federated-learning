#include "includes.h"
//#include "microphone.h"

#ifndef PRE_PROCESSED
#include "fft.h"
#endif
#include "ml.h"
#include "lib/test.h"
#include "stdio.h"

void init();
void simple_train(uint8_t class);

float input[NODES_L0];
float out[NODES_L2];
float target[NODES_L2];
float error;
int main(void)
{
    init();
#ifndef PRE_PROCESSED
    init_mfcc();
    uint16_t recording[NUM_SAMPLES];
#endif
    //_micInit();
    ml_init();
    uint8_t msgtype;
    uint8_t class;

    //GPIO_setAsOutputPin(GPIO_PORT_P2, 0b111);
    srand(RANDOM_SEED);
    while (1)  {
        // if message available on UART
        UART_Read(&msgtype,1);
        switch (msgtype)
            {
            case MODEL_REQUEST:
                UART_Write(&num_epochs, sizeof(num_epochs)); // send number of epochs
                sendModel();                                 // send NN weights
                num_epochs = 0;                              // reset number of epochs to zero
                break;
            case ML_MODEL:
                getModel();
                break;

            case TRAINING_SAMPLE:
                UART_Read(&class,1);
                #ifndef PRE_PROCESSED
                UART_Read(&recording, sizeof(recording));
                feature_extraction(recording, input);
                #else
                UART_Read(&input, sizeof(input));
                #endif

                //UART_Write(&input, NODES_L0*sizeof(float));

                simple_train(class);
                UART_Write(&error,sizeof(error));
                break;
            default:
                UART_Read(&class,1);
                UART_Write(&class,1);
            }
    }
}

void simple_train(uint8_t class)
{
    target[0] = 0;
    target[1] = 0;
    target[2] = 0;
    target[class - 1] = 1.0; // button 1 -> {1,0,0};  button 2 -> {0,1,0};  button 3 -> {0,0,1}

   error = learn(input, out, target); // train

    num_epochs++;
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
        EUSCI_A_UART_8_BIT_LEN
   };

void init()
{
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    /*
    // left button on msp board
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);

    // BUTTON B
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);
    // BUTTON A
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN5);
    // BUTTON J
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN1);
    */

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

    //init_timer();

    UART_Init(UART0Config);
    Interrupt_enableMaster();
}
