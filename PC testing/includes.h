#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h> 

#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#define READ_PROCESSED

#define RECORD_ONLY_MODE

#define LEARNINGRATE 0.6
#define LEARNINGMOMENTUM 0

#define RANDOM_SEED 17762

#define SAMPLE_FREQUENCY 16000
#define SMCLK_FREQUENCY  48000000

#define NUM_MEL_BANDS 32
#define MFCC_COEFF  13
#define NUM_FRAMES  49
#define FFT_WINDOW 320

#define NUM_SAMPLES FFT_WINDOW*NUM_FRAMES

#define NODES_L0 MFCC_COEFF*NUM_FRAMES
#define NODES_L1 25
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
