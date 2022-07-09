#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "lib/LcdDriver/Crystalfontz128x128_ST7735.h"
#include "lib/UART_Driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <arm_math.h>
#include <arm_const_structs.h>

Graphics_Context ctx;


#define NUM_SAMPLES 8192

#define NUM_MEL_BANDS 21
#define MFCC_COEFF  9
#define NUM_FRAMES  32

#define TRAINING_ROUNDS_BEFORE_FL 10

#define NODES_L1 25
#define NODES_L2 3
