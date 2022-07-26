#ifndef __FFT_H__
#define __FFT_H__

#include "includes.h"


volatile arm_status status;

#define DO_BIT_REVERSE 0
#define FFT_FORWARD_TRANSFORM 0




void feature_extraction (int16_t *s, float *out){
    uint32_t i, j, k;
    union {
        int16_t raw_fft[FFT_WINDOW * 2];
        float   log[FFT_WINDOW];
    } temp_data;

    for (i=0; i<NUM_FRAMES; i++){
       arm_rfft_instance_q15 instance;
       status = arm_rfft_init_q15(&instance, FFT_WINDOW, FFT_FORWARD_TRANSFORM, DO_BIT_REVERSE);

       arm_rfft_q15(&instance, s+i*FFT_WINDOW, temp_data.raw_fft);

       //calculate magnitude in log scale
       for(j = 0; j < FFT_WINDOW * 2; j += 2) {
           temp_data.log[ j / 2 ] = log((sqrtf((temp_data.raw_fft[j]     * temp_data.raw_fft[j]) +
                                               (temp_data.raw_fft[j + 1] * temp_data.raw_fft[j + 1])))+1);
       }


       //convert to log frequency scale
       float mel[NUM_MEL_BANDS];
       int16_t prev_band = 0;
       int16_t curr_band = 1;
       int16_t next_band = 3;
       for (j = 1; next_band < FFT_WINDOW; j++){
           float temp = 0;
           for (k = prev_band; k < next_band; k++){
               if (k < curr_band)
                   temp += temp_data.log[k]*(curr_band-k)*(1.0/j);
               else
                   temp += temp_data.log[k]*(k-curr_band)*(1.0/j);
          }
           mel[j]=temp;
           prev_band = curr_band;
           curr_band = next_band;
           next_band += j;
       }


       //calculate first MFCC_COEFF of MFCC with DCT
       for (k = 0; k < MFCC_COEFF; ++k) {
           float sum = 0.;
           float s = (k == 0) ? sqrt(.5) : 1.;
           for (j = 0; j < NUM_MEL_BANDS; ++j) {
             sum += s * mel[j] * cos(M_PI * (j + .5) * k / NUM_MEL_BANDS);
           }
           out[k+MFCC_COEFF*i] =  1.0 / (1.0 + exp(-sum * sqrt(2.0 / NUM_MEL_BANDS)));
         }

    }
}

#endif
