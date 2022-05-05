#ifndef __FFT_H__
#define __FFT_H__

#include "includes.h"
#include "libmfcc.h"

#define FFT_WINDOW 320
float hann[FFT_WINDOW];
volatile arm_status status;

void fft_init(){
    int n;
    for(n = 0; n < FFT_WINDOW; n++) {
        hann[n] = 0.5f - 0.5f * cosf((2 * PI * n) / (SAMPLE_LENGTH - 1));
    }
}

void feature_extraction (int16_t *s, float *out){
    int i, j, coeff;
    int16_t data_input[FFT_WINDOW * 2];
    float data_temp[FFT_WINDOW];
    for (i=0; i<32; i++){
       /*for(j = 0; j < FFT_WINDOW; j++){
           s[j+i*FFT_WINDOW] = (int16_t)(hann[j] * s[j+i*FFT_WINDOW]);
       }*/

       arm_rfft_instance_q15 instance;
       status = arm_rfft_init_q15(&instance, FFT_WINDOW, 0, 1);

       arm_rfft_q15(&instance, s+i*FFT_WINDOW, data_input);

       //calculate magnitude
       for(j = 0; j < FFT_WINDOW * 2; j += 2) {
           data_temp[ j / 2 ] =(int32_t)(sqrtf((data_input[j] * data_input[j]) +
                                               (data_input[j + 1] * data_input[j + 1])));
       }
       // Compute the first 13 coefficients of MFCC
       for(coeff = 0; coeff < 13; coeff++)   {
           out[coeff+i*13] = GetCoefficient(data_temp, 16000, 20, FFT_WINDOW, coeff);
       }
    }
}

#endif

