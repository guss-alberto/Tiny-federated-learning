#ifndef __MICROPHONE_H__
#define __MICROPHONE_H__

#include "includes.h"

#define SAMPLE_FREQUENCY 8000 //16383
#define SMCLK_FREQUENCY  48000000

/* DMA Control Table */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(MSP_EXP432P401RLP_DMAControlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (1024)))
#elif defined(__CC_ARM)
__align(1024)
#endif
static DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[32];
/* Graphic library context */
Graphics_Context g_sContext;


int8_t mic_status;
void _micInit();
void micSample(int16_t *dst, uint32_t len);

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmConfig =
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    (SMCLK_FREQUENCY / SAMPLE_FREQUENCY),
    TIMER_A_CAPTURECOMPARE_REGISTER_1,
    TIMER_A_OUTPUTMODE_SET_RESET,
    (SMCLK_FREQUENCY / SAMPLE_FREQUENCY) / 2
};


void _micInit(){
    // Configuring Timer_A0 to period of ~500ms and duty cycle 10%  (3200 ticks)
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    // Initializing ADC (MCLK/1/1)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);

    // trigger on clock
    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    // set boosterpack microphone pin as ADC input
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_TERTIARY_MODULE_FUNCTION);

    // configuring ADC
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A10, false);

    // Set ADC result format to signed binary
    ADC14_setResultFormat(ADC_SIGNED_BINARY);

    // configuring DMA
    DMA_enableModule();
    DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);

    DMA_disableChannelAttribute(DMA_CH7_ADC14,
                                UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                UDMA_ATTR_HIGH_PRIORITY |
                                UDMA_ATTR_REQMASK);

    /* Assigning/Enabling Interrupts */
    DMA_assignInterrupt(DMA_INT1, 7);
    Interrupt_enableInterrupt(INT_DMA_INT1);
    DMA_assignChannel(DMA_CH7_ADC14);
    Interrupt_enableMaster();
}

void micSample(int16_t *dst, uint32_t len){
    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 0
     *  and the destination to the
     * destination data array. */
    DMA_setChannelControl(
        UDMA_PRI_SELECT | DMA_CH7_ADC14,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
        UDMA_DST_INC_16 | UDMA_ARB_1);
    DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
                           UDMA_MODE_PER_SCATTER_GATHER, (void*) &ADC14->MEM[0],
                               dst, len);

    /* Assigning/Enabling Interrupts */
    DMA_assignInterrupt(DMA_INT1, 7);
    Interrupt_enableInterrupt(INT_DMA_INT1);
    DMA_assignChannel(DMA_CH7_ADC14);
    DMA_clearInterruptFlag(7);
    Interrupt_enableMaster();

    /* Now that the DMA is primed and setup, enabling the channels. The ADC14
     * hardware should take over and transfer/receive all bytes */
    DMA_enableChannel(7);
    ADC14_enableConversion();

    //wait until done
    while (/*mic_status ==*/ 1);

    //disable everything
    DMA_disableChannel(7);
    ADC14_disableConversion();

}

#define SAMPLE_LENGTH 512
int16_t data_array1[SAMPLE_LENGTH];
int16_t data_array2[SAMPLE_LENGTH];


/* Completion interrupt for ADC14 MEM0 */
void DMA_INT1_IRQHandler(void)
{
    mic_status++;
    char str[7];
    sprintf(str, "%d", mic_status);
    Graphics_drawString(& g_sContext, str, -1, 20, 30, true);
}


#endif
