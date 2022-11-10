#ifndef __FFT_H__
#define __FFT_H__

#include "includes.h"
#include "lib/fft.h"

static float mel_to_frequency(float mel) {
    return 700.0f * (exp(mel / 1127.0f) - 1.0f);
}
static float frequency_to_mel(float f) {
    return 1127.0 * log(1 + f / 700.0f);
}
uint16_t freq_index[NUM_MEL_BANDS + 2];

void init_mfcc(){
    float mel[NUM_MEL_BANDS+2], hertz[NUM_MEL_BANDS+2];
    uint16_t i;
    float low = frequency_to_mel (LOW_FREQ);
    float high = frequency_to_mel (HIGH_FREQ);

    for (i=0; i<NUM_MEL_BANDS+2; i++){
        mel[i] = low + i*(high-low)/NUM_MEL_BANDS;
    }

    for (i = 0; i < NUM_MEL_BANDS + 2; i++) {
        hertz[i] = mel_to_frequency(mel[i]);
        if (hertz[i] < LOW_FREQ) {
            hertz[i] = LOW_FREQ;
        }
        if (hertz[i] > HIGH_FREQ) {
            hertz[i] = HIGH_FREQ;
        }
    }
    for (i = 0; i < NUM_MEL_BANDS + 2; i++) {
        freq_index[i] = (int)(floor((FFT_WINDOW*2+1) * hertz[i] / SAMPLE_FREQUENCY));
    } 
}


void feature_extraction (int16_t *s, float *out){
    uint32_t i, j, k;

    int16_t fft[FFT_WINDOW * 2];


    for (i=0; i<NUM_FRAMES; i++){
        //preemphasis
        
        for (j=PRE_SHIFT; j<FRAME_SIZE; j++){
            fft[j-PRE_SHIFT] = s[i*FRAME_SIZE+j] - PRE_COEFF*s[i*FRAME_SIZE+j-PRE_SHIFT];
        }
        
            
        //zero-pad to window
        for (j=FRAME_SIZE-PRE_SHIFT; j<FFT_WINDOW*2; j++){
            fft[j] = 0;
        }

        fix_fftr(fft, 9, 0);
        float nfft[FFT_WINDOW];
        for (j=0; j<FFT_WINDOW; j++){
            nfft[j] = (sqrt(fft[i]*fft[i] + fft[i+FFT_WINDOW/2]*fft[i+FFT_WINDOW/2])+32768)/32768;
        }
        
        //calculate mel filterbanks 
        float mel[NUM_MEL_BANDS];
        
        for (j = 0; j < NUM_MEL_BANDS; j++){
            int left = freq_index[j];
            int middle = freq_index[j + 1];
            int right = freq_index[j + 2];

            float temp;
            for (k = left+1; k<middle; k++){
                temp+=nfft[k]*((float)(k-left)/(float)(middle-left));
            }
            for (k = middle; k<right; k++){
                temp+=nfft[k]*(1-(float)(k-middle)/(float)(right-middle));
            }

            mel[j] = temp/(left-right);
        }

        //calculate first MFCC_COEFF of MFCC with DCT
        for (k = 0; k < MFCC_COEFF; ++k) {
           float sum = 0.0;
           float s = (k == 0) ? sqrt(.5) : 1.;
           for (j = 0; j < NUM_MEL_BANDS; ++j) {
             sum += s * mel[j] * cos(M_PI * (j + .5) * k / NUM_MEL_BANDS);
           }
           out[k+MFCC_COEFF*i] =  1.0 / (1.0 + exp(-sum * sqrt(2.0 / NUM_MEL_BANDS)));
       }

    }
}

#endif
