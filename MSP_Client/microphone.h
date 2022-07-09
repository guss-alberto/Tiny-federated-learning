#ifndef __MICROPHONE_H__
#define __MICROPHONE_H__

#include "includes.h"

#define SAMPLE_FREQUENCY 16000
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

    ADC14_enableInterrupt(ADC_INT0);
    Interrupt_enableInterrupt(INT_ADC14);
    Interrupt_enableMaster();
}

int16_t *sample_dst;
uint32_t sample_num;

void micSample(int16_t *dst, uint32_t len){
    sample_dst = dst;
    sample_num = 0;

    ADC14_enableConversion();

    //wait until done
    while (sample_num < len);

    ADC14_disableConversion();
}

void ADC14_IRQHandler(void)
{
    uint64_t status;
    status = ADC14_getEnabledInterruptStatus();
    /* ADC_MEM0 conversion completed */
    if(status & ADC_INT0)
    {
        /* Store ADC14 conversion results */
        sample_dst[sample_num++] = ADC14_getResult(ADC_MEM0);
    }


    ADC14_clearInterruptFlag(status);
}
#endif
