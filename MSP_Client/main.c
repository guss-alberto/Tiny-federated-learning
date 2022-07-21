#include "includes.h"
#include "microphone.h"
#include "fft.h"
#include "ml.h"


void init();
uint8_t buttons();



uint8_t mode = MODE_EVAL;

union { //use union to save memory
    int16_t rec[NUM_SAMPLES];
    struct {
        float input[NODES_L0];
        float out[NODES_L2];
        float target[NODES_L2];
        float error;
    } ml;
} myData;

int main(void)
{
    init();
    uint8_t a;
    Graphics_drawString(&ctx, (int8_t*)modeStr[(mode&MODE_S) - 1], 20, 20, 10, true);

    while (1){
        if (UART_Read_nb(&a, 1)){
            switch (a){
            case FL_READY:
                UART_Write(&a,1);   //send message back to indicate ready status
                UART_Write((void*)&num_epochs,sizeof(num_epochs)); //send number of epochs
                sendModel();        //send NN weights
                getModel();         //wait to receive new model
                num_epochs=0;       //reset number of epochs to zero
                break;
            case CHANGE_MODE:
                UART_Read(&mode, 1);  //update mode value
                Graphics_clearDisplay(&ctx); //reset screen
                Graphics_drawString(&ctx, (int8_t*)modeStr[(mode&MODE_S) - 1], 20, 20, 10, true);
                break;
            }
        }

        switch (mode & MODE_S){
        case MODE_LEARN:
            if (mode & MODE_REMOTE_TRAIN ){
                a=REQUEST_TRAINING_DATA;
                UART_Write(&a,1);   //send request for training data
                UART_Read(&a,1);    //read response before
                if ( a != 0 ) {
                    mode = a;
                    Graphics_clearDisplay(&ctx); //reset screen
                    Graphics_drawString(&ctx, (int8_t*)modeStr[(mode&MODE_S) - 1], 20, 20, 10, true);
                    break;
                }
                UART_Read((void*)myData.ml.input, sizeof(myData.ml.input)); //Wait for data input
                UART_Read(&a, 1); //get output class
            } else {
               a = buttons();
               if (!a) break; // if no buttons are pushed exit and continue the loop
               Graphics_drawString(&ctx, "sampling....     ", 20, 20, 20, true);
               micSample(myData.rec);
               Graphics_drawString(&ctx, "processing...", 20, 10, 20, true);
               feature_extraction(myData.rec, myData.ml.input);
            }

            myData.ml.target[0] = 0;
            myData.ml.target[1] = 0;
            myData.ml.target[2] = 0;
            myData.ml.target[a-1] = 1.0; // button 1 -> {1,0,0};  button 2 -> {0,1,0};  button 3 -> {0,0,1}

            myData.ml.error = learn (myData.ml.input, myData.ml.out, myData.ml.target);
            UART_Write((void*)&myData.ml.error, sizeof(float));
            num_epochs++;
            break;
        case MODE_EVAL:
            a = buttons();
            if (!a) break; // if no buttons are pushed exit and continue the loop
            Graphics_drawString(&ctx, "sampling....     ", 20, 10, 20, true);
            micSample(myData.rec);
            Graphics_drawString(&ctx, "processing...", 20, 10, 20, true);
            feature_extraction(myData.rec, myData.ml.input);

            eval (myData.ml.input, myData.ml.out);
            char str[30];
            sprintf(str, "%.2f | %.2f | %.2f", myData.ml.out[0], myData.ml.out[1], myData.ml.out[2]);

            Graphics_drawString(&ctx, "DONE......", 20, 10, 20, true);
            Graphics_drawString(&ctx, (int8_t*)str, 30, 10, 30, true);
            break;
        case MODE_RECORD:
            a=buttons();
            if (a){
                Graphics_drawString(&ctx, "sampling....     ", 20, 10, 20, true);
                micSample(myData.rec);
                Graphics_drawString(&ctx, "processing...", 20, 10, 20, true);
                feature_extraction(myData.rec, myData.ml.input);
                const char tmp = DATA_READY;
                UART_Write((void*)tmp, 1); //send ready message
                UART_Write((void*)myData.ml.input, sizeof(myData.ml.input));
                UART_Write((void*)a, 1); //send class
                Graphics_drawString(&ctx, "DONE......", 20, 10, 20, true);
            }
            break;
        }
    }
}

const eUSCI_UART_ConfigV1 UART0Config =
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     13,
     0,
     37,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

uint8_t buttons (){
    uint8_t a = 0;
    if (!GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN5)&&!a){ //A
        a = 1;
        Graphics_drawString(&ctx, "Release to record", 20, 10, 20, true);
        while(!GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN5));
    }
    if (!GPIO_getInputPinValue(GPIO_PORT_P5,GPIO_PIN1)&&!a){ //B
        a = 2;
        Graphics_drawString(&ctx, "Release to record", 20, 10, 20, true);
        while(!GPIO_getInputPinValue(GPIO_PORT_P5,GPIO_PIN1));
    }
    if (!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN1)&&!a){ //J
        a = 3;
        Graphics_drawString(&ctx, "Release to record", 20, 10, 20, true);
        while (!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN1));
    }
    return a;
}

void init(){
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    //BUTTON B
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5,GPIO_PIN1);
    //BUTTON A
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3,GPIO_PIN5);
    //BUTTON J
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4,GPIO_PIN1);

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

    UART_Init(EUSCI_A0_BASE, UART0Config);
    _micInit();
    ml_init();
}




