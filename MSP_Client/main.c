#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

void main(void) {
    WDT_A_holdTimer();
    Interrupt_disableMaster();
}
