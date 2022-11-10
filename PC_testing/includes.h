#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h> 
#include <stdbool.h> 

#ifndef __INCLUDES_H__
#define __INCLUDES_H__

//#define READ_PROCESSED


#define LEARNINGRATE 0.6
#define LEARNINGMOMENTUM 0

#define RANDOM_SEED 17762

#define SAMPLE_FREQUENCY 16000
#define SMCLK_FREQUENCY  48000000


#define HIGH_FREQ  SAMPLE_FREQUENCY/2.0f 
#define LOW_FREQ   300.0f

#define PRE_COEFF 0.970f
#define PRE_SHIFT 1
#define FRAME_SIZE 320

#define NUM_MEL_BANDS 32
#define MFCC_COEFF  13
#define NUM_FRAMES  50
#define FFT_WINDOW 512

#define NUM_SAMPLES FRAME_SIZE*NUM_FRAMES

#define NODES_L0 MFCC_COEFF*NUM_FRAMES
#define NODES_L1 25
#define NODES_L2 3


#endif
