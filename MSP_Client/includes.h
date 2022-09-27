#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "lib/LcdDriver/Crystalfontz128x128_ST7735.h"
#include "lib/UART_Driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <arm_math.h>
#include <arm_const_structs.h>

#ifndef __INCLUDES_H__
#define __INCLUDES_H__

Graphics_Context ctx;

#define RECORD_ONLY_MODE


#define RANDOM_SEED 17762

#define SAMPLE_FREQUENCY 16000
#define SMCLK_FREQUENCY  48000000

#define NUM_MEL_BANDS 22
#define MFCC_COEFF  9
#define NUM_FRAMES  28
#define FFT_WINDOW 256

#define NUM_SAMPLES FFT_WINDOW*NUM_FRAMES

#define NODES_L0 MFCC_COEFF*NUM_FRAMES
#define NODES_L1 24
#define NODES_L2 3

#define FL_READY     0x01
#define FL_SEND_DATA 0x02
#define REQUEST_TRAINING_DATA 0x03
#define DATA_READY 0x04
#define RECEIVE_TRAINING 0x05
#define RAW_DATA_READY 0x06
#define CHANGE_MODE 0xff

#define MODE_LEARN  0x01
#define MODE_EVAL   0x02
#define MODE_RECORD 0x03

const int8_t *modeStr[] = {"Training mode",
                           "Evaluating mode",
                           "Data recording"};

#endif
