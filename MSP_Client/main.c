#include "includes.h"
#include "microphone.h"
#include "fft.h"
#include "ml.h"


void init();


#define MODE_LEARN 1
#define MODE_EVAL  0
#define MODE_FL    0b01
#define REMOTE_DATA 0b001

int8_t mode = MODE_LEARN & MODE_FL;

int main(void)
{
    init();
    int16_t rec[NUM_SAMPLES];

    Graphics_drawString(&ctx, "sampling...", 10, 10, 10, true);
    micSample(rec, NUM_SAMPLES);

    Graphics_drawString(&ctx, "processing...", 20, 10, 10, true);
    UART_Write(EUSCI_A0_BASE, (void*)rec, sizeof(rec));
    float out[MFCC_COEFF*NUM_FRAMES];
    feature_extraction (rec, out);

    Graphics_drawString(&ctx, "sending...", 20, 10, 10, true);
    UART_Write(EUSCI_A0_BASE, (void*)out, sizeof(out));

    float res[NODES_L2];
    Graphics_drawString(&ctx, "evaluating...", 30, 10, 10, true);
    eval (out, res);
    Graphics_drawString(&ctx, "done!        ", 30, 10, 10, true);
    char str[20];
    sprintf(str, "%.2f - %.2f - %.2f", res[0], res[1], res[2]);
    Graphics_drawString(&ctx, (int8_t*)str, 20, 10, 30, true);
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


void init(){
    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    /* Set the core voltage level to VCORE1 */
    PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
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

/* Cortex-M4 Processor Exceptions */
void HardFault_Handler  () {Graphics_drawString(&ctx, "HardFault_Handler ", 10, 10, 50, true);}
