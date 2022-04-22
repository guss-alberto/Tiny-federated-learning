#include "microphone.h"

uint16_t *_micBuffer;
uint16_t  _numSamples;


void _micInit(){
    ADC14_enableModule();

    // Configures Pin 4.3 as ADC input
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_TERTIARY_MODULE_FUNCTION);

    // Initializing ADC (SMCLK = 48MHz)
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);


    // Set reference voltage to 2.5 V and ADC input A10 (microphone input on pin 4.3)
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A10, ADC_NONDIFFERENTIAL_INPUTS);

    // Use single memory location and repeat after each
    ADC14_configureSingleSampleMode(ADC_MEM0, 1);

    // Enable interrupt on conversion on channel 0 complete
    ADC14_enableInterrupt(ADC_INT0);
    Interrupt_enableInterrupt(INT_ADC14);
}

void recordSample(uint16_t* buffer){
    _numSamples = NUM_AUDIO_SAMPLES;
    /* Setting up the sample timer to automatically step through the sequence
     * convert.
     */
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /* Triggering the start of the sample */
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();
}

void ADC14_IRQHandler(void){
    uint64_t status;
    // Clear interrupt flags
    status = ADC14_getEnabledInterruptStatus();
    ADC14_clearInterruptFlag(status);

    // Add to the mic buffer the current result
    *_micBuffer++ = ADC14_getResult(ADC_MEM0);

    // When we run out of samples to take, disable ADC
    if (--_numSamples == 0){
        while (!ADC14_disableModule()); // try until current conversion is done
    }


}
