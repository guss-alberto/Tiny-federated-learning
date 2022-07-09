#ifndef __FFT_H__
#define __FFT_H__

#include "includes.h"

#define FFT_WINDOW NUM_SAMPLES/NUM_FRAMES
volatile arm_status status;

#define DO_BIT_REVERSE 1
#define FFT_FORWARD_TRANSFORM 0

void feature_extraction (int16_t *s, float *out){
    uint32_t i, j, k;
    int16_t fft_data[FFT_WINDOW * 2];
    double data_temp[FFT_WINDOW];
    for (i=0; i<NUM_FRAMES; i++){
       arm_rfft_instance_q15 instance;
       status = arm_rfft_init_q15(&instance, FFT_WINDOW, FFT_FORWARD_TRANSFORM, DO_BIT_REVERSE);

       arm_rfft_q15(&instance, s+i*FFT_WINDOW, fft_data);

       //calculate magnitude in log scale
       for(j = 0; j < FFT_WINDOW * 2; j += 2) {
           data_temp[ j / 2 ] = log((sqrtf((fft_data[j] * fft_data[j]) +
                                           (fft_data[j + 1] * fft_data[j + 1]))));
       }


       //convert to log frequency scale
       float mel[NUM_MEL_BANDS];
       int16_t band_size = 1;
       int16_t prev_band = 0;
       int16_t curr_band = 1;
       int16_t next_band = 3;
       for (j = 0; next_band < FFT_WINDOW; j++){
           for (k = prev_band; k < next_band; k++){
               if (k < curr_band)
                   mel[j] += data_temp[k]*(curr_band-k)*(1.0/band_size);
               else
                   mel[j] += data_temp[k]*(k-curr_band)*(1.0/band_size);
          }
           prev_band = curr_band;
           curr_band = next_band;
           next_band += band_size;
           band_size++;
       }


       //calculate first MFCC_COEFF of MFCC with DCT
       for (k = 0; k < MFCC_COEFF; ++k) {
           float sum = 0.;
           float s = (k == 0) ? sqrt(.5) : 1.;
           for (j = 0; j < NUM_MEL_BANDS; ++j) {
             sum += s * mel[j] * cos(M_PI * (j + .5) * k / NUM_MEL_BANDS);
           }
           out[k+MFCC_COEFF*i] = sum * sqrt(2. / NUM_MEL_BANDS);
         }

    }
}

#endif
