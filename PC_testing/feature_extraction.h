#ifndef __FFT_H__
#define __FFT_H__

#include "includes.h"
#include "lib/fft.h"
uint16_t freq_index[NUM_MEL_BANDS + 2];

#define PAD_SIZE (CMNW_WIN_SIZE-1)/2


void pad_symmetric (float* in, float* out);
static float mel_to_frequency(float mel) {
    return 700.0f * (exp(mel / 1127.0f) - 1.0f);
}
static float frequency_to_mel(float f) {
    return 1127.0 * log(1 + f / 700.0f);
}

void init_mfcc(){
    float mel[NUM_MEL_BANDS+2], hertz[NUM_MEL_BANDS+2];
    uint16_t i;
    float low  = frequency_to_mel (LOW_FREQ);
    float high = frequency_to_mel (HIGH_FREQ);

    //linear 
    for (i=0; i<NUM_MEL_BANDS+2; i++){
        mel[i] = low + i*(high-low)/(NUM_MEL_BANDS+1);
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
        freq_index[i] = (int)(floor((FFT_WINDOW/2+2) * hertz[i] / SAMPLE_FREQUENCY))-1;
    } 
}


void feature_extraction (int16_t *s, float *out){
    uint32_t i, j, k;

    for (i=0; i<NUM_FRAMES; i++){
        //preemphasis
        int16_t rfft[FFT_WINDOW];
        int16_t ifft[FFT_WINDOW];
        for (j=PRE_SHIFT; j<FRAME_SIZE; j++){
            rfft[j-PRE_SHIFT] = s[i*FRAME_SIZE+j] - PRE_COEFF*s[i*FRAME_SIZE+j-PRE_SHIFT];
        }
        
            
        //zero-pad to window
        memset(rfft+FRAME_SIZE, 0, FFT_WINDOW-FRAME_SIZE);
        memset(ifft, 0, FFT_WINDOW);

        fix_fft(rfft, ifft, 9, 0);
        float nfft[FFT_WINDOW];
        float energy  =  0;
        for (j=0; j<FFT_WINDOW; j++){
            nfft[j] = log(((rfft[j]*rfft[j] + ifft[j]*ifft[j])/32768.0f)+0.0000001);
            energy += nfft[j];
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

            mel[j] = (temp/(left-right)+1);
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
        
        // DCT_NORMALIZATION_ORTHO
        out[MFCC_COEFF*i] = out[MFCC_COEFF*i] * sqrt(1.0f / (float)(4 * NUM_MEL_BANDS));
		for (j = 1; j < MFCC_COEFF; j++) {
			out[j+MFCC_COEFF*i] = out[j+MFCC_COEFF*i] * sqrt(1.0f / (float)(2 * NUM_MEL_BANDS));
		}

        //replace DC coefficient with frame energy
        out[k+MFCC_COEFF*i] = energy;
    }

    //cmnw

    //window mean subtraction
    float padded[NUM_FRAMES+CMNW_WIN_SIZE][MFCC_COEFF];
    pad_symmetric(out, *padded);
    for (i=0; i<NUM_FRAMES; i++){
        for (k=0; k<MFCC_COEFF; k++){
            float sum = 0;
            for (j=i; j<i+CMNW_WIN_SIZE; j++){
                sum += padded[i][j];
            }
            out[k+MFCC_COEFF*i] -= sum/CMNW_WIN_SIZE;
        }
    }

    //variance normalization
    pad_symmetric(out, *padded);
    
    for (i=0; i<NUM_FRAMES; i++){
        float means[MFCC_COEFF] = {0};
        for (k=0; k<MFCC_COEFF; k++){
            float temp = 0;
            float mean = 0;
            for (j=i; j<i+CMNW_WIN_SIZE; j++){
                mean += padded[i][j];
            }
            mean /= CMNW_WIN_SIZE;
            for (j=i; j<i+CMNW_WIN_SIZE; j++){
                temp += (padded[i][j]-mean)*(padded[i][j]-mean);
            }
            out[k+MFCC_COEFF*i] /= sqrt(temp/CMNW_WIN_SIZE);
        }
    }
    
}

void pad_symmetric (float* in, float* out){
    uint32_t pad_before_index = 0;
    bool pad_before_direction_up = true;
    
    for (int32_t ix = PAD_SIZE - 1; ix >= 0; ix--) {
            memcpy(out + (MFCC_COEFF * ix),
                in + (pad_before_index * MFCC_COEFF),
                MFCC_COEFF * sizeof(float));

            if (pad_before_index == 0 && !pad_before_direction_up) {
                pad_before_direction_up = true;
            }
            else if (pad_before_index == NUM_FRAMES - 1 && pad_before_direction_up) {
                pad_before_direction_up = false;
            }
            else if (pad_before_direction_up) {
                pad_before_index++;
            }
            else {
                pad_before_index--;
            }
        }

    memcpy(out + (MFCC_COEFF * PAD_SIZE),
            in,
            NUM_FRAMES * MFCC_COEFF * sizeof(float));

    int32_t pad_after_index = NUM_FRAMES - 1;
    bool pad_after_direction_up = false;

    for (int32_t ix = 0; ix < PAD_SIZE; ix++) {
        memcpy(out + (MFCC_COEFF * (ix + PAD_SIZE + NUM_FRAMES)),
            in + (pad_after_index * MFCC_COEFF),
            MFCC_COEFF * sizeof(float));

        if (pad_after_index == 0 && !pad_after_direction_up) {
            pad_after_direction_up = true;
        }
        else if (pad_after_index == NUM_FRAMES - 1 && pad_after_direction_up) {
            pad_after_direction_up = false;
        }
        else if (pad_after_direction_up) {
            pad_after_index++;
        }
        else {
            pad_after_index--;
        }
    }
}

#endif
