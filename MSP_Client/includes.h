#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "lib/LcdDriver/Crystalfontz128x128_ST7735.h"
#include "lib/UART_Driver.h"
#include "lib/LoRa/LoRa.h"
#include <stdio.h>
#include <stdlib.h>
#include <arm_math.h>
#include <arm_const_structs.h>

#ifndef __INCLUDES_H__
#define __INCLUDES_H__

Graphics_Context ctx;

#define RECEIVERMODE
#define TEST_SENDER
#define DEVICE_ID 1

#define RANDOM_SEED 17762

#define USE_MOMENTUM


#define LEARNINGRATE 0.6
#define LEARNINGMOMENTUM 0.9
#define NODES_L1 20
#define NODES_L2 3

#define RANDOM_SEED 17762

#define SAMPLE_FREQUENCY 16000
#define SMCLK_FREQUENCY  48000000

//feature extraction parameters
#define MFCC_COEFF  13
#define FRAME_SIZE 320
#define NUM_FRAMES  50
#define FFT_WINDOW 512

//highest and lowest freuencies for mel filters
#define NUM_MEL_BANDS 32
#define LOW_FREQ   300.0f
#define HIGH_FREQ  SAMPLE_FREQUENCY/2.0f

//preemphasis
#define PRE_COEFF 0.970f
#define PRE_SHIFT 1

#define CMNW_WIN_SIZE 101 //must be odd number


#define NUM_SAMPLES FRAME_SIZE*NUM_FRAMES
#define NODES_L0 MFCC_COEFF*NUM_FRAMES


#define BROADCAST    0xff
#define FL_READY     0x01
#define FL_SEND_DATA 0x02
#define REQUEST_TRAINING_DATA 0x03
#define DATA_READY 0x04
#define RECEIVE_TRAINING 0x05
#define RAW_DATA_READY 0x06
#define NEW_MODEL 0x07

#define MODE_LEARN  0x01
#define MODE_EVAL   0x02
#define MODE_RECORD 0x03

#endif
