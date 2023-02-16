/*
 * test.h
 *
 *  Created on: 12 Feb 2023
 *      Author: guss
 */

#ifndef LIB_TEST_H_
#define LIB_TEST_H_

const Timer_A_ContinuousModeConfig continuousModeConfig = {
        TIMER_A_CLOCKSOURCE_ACLK,           // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_64,     // ACLK/64 = 1 kHz
        TIMER_A_TAIE_INTERRUPT_ENABLE,      // Enable Overflow ISR
        TIMER_A_DO_CLEAR                    // Clear Counter
};


void init_timer (){
    /* initializes ACLK to 64 kHz*/
    CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_2);

    //setup clock for millis and sleep function
    Timer_A_configureContinuousMode(TIMER_A2_BASE, &continuousModeConfig);
    /* Enabling interrupts and going to sleep */
    Interrupt_enableInterrupt(INT_TA2_N);
    /* Starting the Timer_A2 in continuous mode */
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_CONTINUOUS_MODE);
}

uint16_t millisHI = 0;                      //high 16 bits of millisecond counter
uint32_t millis(){                          //concurrency problem and freezes every once in a while
    uint16_t _millisHI, _millisLO;
    do {
        _millisHI = millisHI;
        _millisLO = Timer_A_getCounterValue(TIMER_A2_BASE);
    } while (_millisHI != millisHI);             //if timer overflow is triggered after the values are stored, reupdate everything
                                                 //this should prevent cases where the counter is behind by 2^16
    return _millisHI<<16|_millisLO;                  //add the high bits to make a full 32 bit
}

void TA2_N_IRQHandler(void){
    Timer_A_clearInterruptFlag(TIMER_A2_BASE);
    millisHI++;
}



#endif /* LIB_TEST_H_ */
