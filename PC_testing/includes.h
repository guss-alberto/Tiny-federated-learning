#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h> 
#include <stdbool.h> 
#include <string.h>

#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#define READ_PROCESSED
#define USE_MOMENTUM


#define LEARNINGRATE 0.6
#define LEARNINGMOMENTUM 0.9
#define NODES_L1 25
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

#endif
