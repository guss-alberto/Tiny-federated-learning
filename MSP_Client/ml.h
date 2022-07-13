#ifndef __ML_H__
#define __ML_H__

#include "includes.h"

#define NODES_L0 MFCC_COEFF*NUM_FRAMES

float LearningRate = 0.3;
float Momentum = 0.9;

float weights_L1[NODES_L1][NODES_L0+1];
float change_L1 [NODES_L1][NODES_L0+1];

float weights_L2[NODES_L2][NODES_L1+1];
float change_L2 [NODES_L2][NODES_L1+1];

//initializes the weight values to random
void ml_init();

//uses the neural network model to classify the input
void eval (float *input, float *out);

//backpropagates error and updates, return value is error
float learn (float *input, float *out, float *target);

//sends or get the model data over UART
void sendModel ();
void getModel ();

void ml_init(){
    uint16_t i, j;
    for (i=0; i<NODES_L1; i++){
        for (j=0; j<=NODES_L0; j++){
            weights_L1[0][j] = (2.0*(float)rand()/RAND_MAX)-1.0;
            change_L1[i][j] = 0;
        }
    }
    for (i=0; i<NODES_L2; i++){
        for (j=0; j<=NODES_L1; j++){
            weights_L2[i][j] = (2.0*(float)rand()/RAND_MAX)-1.0;
            change_L2[i][j] = 0;
        }
    }
}

void eval (float *input, float *out){
    float hiddenLayer[NODES_L1], temp;
    int i, j;

    //compute hidden layer
    for (i=0; i<NODES_L1; i++){
        temp = weights_L1[i][0];
        for (j=0; j<NODES_L0; j++){
            temp += weights_L1[i][j+1]*input[j];
        }
        hiddenLayer[i] = 1.0 / (1.0 + exp(-temp));
    }

    //compute output layer
    for (i=0; i<NODES_L2; i++){
        temp=weights_L2[i][0];
        for (j=0; j<NODES_L1; j++){
            temp += weights_L2[i][j+1]*hiddenLayer[j];
        }
        out[i] = 1.0 / (1.0 + exp(-temp));
    }
}

float learn (float *input, float *out, float *target){
   float hiddenLayer[NODES_L1], temp, error;
   float delta_L1[NODES_L1];
   float delta_L2[NODES_L2];

   int i, j;

   //compute hidden layer
   for (i=0; i<NODES_L1; i++){
       temp = weights_L1[i][0];
       for (j=0; j<NODES_L0; j++){
           temp += weights_L1[i][j+1]*input[j];
       }
       hiddenLayer[i] = 1.0 / (1.0 + exp(-temp));
   }

   //compute output layer
   for (i=0; i<NODES_L2; i++){
       temp = weights_L2[i][0];
       for (j=0; j<NODES_L1; j++){
           temp += weights_L2[i][j+1]*hiddenLayer[j];
       }
       out[i] = 1.0 / (1.0 + exp(-temp));

       //compute delta for final layer and error
       delta_L2[i] = (target[i] - out[i]) * out[i] * (1.0 - out[i]);
       error += (1/NODES_L2) * (target[i] - out[i]) * (target[i] - out[i]);
   }

   //backpropagation of errors
   for(i=0; i < NODES_L1; i++) {
       temp = 0;
       for(j=0; j < NODES_L2; j++) {
           temp += weights_L2[i][j+1]*delta_L2[j] ;
       }
       delta_L1[i] = temp * hiddenLayer[i] * (1.0 - hiddenLayer[i]) ;
   }


   //update hidden weigfhts
   for(i=0; i < NODES_L1; i++) {
       change_L1[i][0] = LearningRate * delta_L1[i] + Momentum * change_L1[i][0];
       weights_L1[i][0] += change_L1[i][0];
       for(j = 0 ; j < NODES_L0 ; j++ ) {
           change_L1 [i][j+1] = LearningRate * input[j] * delta_L1[i] + Momentum * change_L1[i][j+1];
           weights_L1[i][j+1] += change_L1[i][j+1] ;
       }
   }


   //update output weights
   for(i=0; i < NODES_L2; i++) {
       change_L2[i][0] = LearningRate * delta_L2[i] + Momentum * change_L2[i][0];
       weights_L2[i][0] += change_L2[i][0];
       for(j = 0 ; j < NODES_L1 ; j++ ) {
           change_L2 [i][j+1] = LearningRate * input[j] * delta_L2[i] + Momentum * change_L2[i][j+1];
           weights_L2[i][j+1] += change_L2[i][j+1] ;
       }
   }
   return error;
}


void sendModel (){
    UART_Write((void*)weights_L1, sizeof(weights_L1));
    UART_Write((void*)weights_L2, sizeof(weights_L2));
}

void getModel (){
    UART_Read((void*)weights_L1, sizeof(weights_L1));
    UART_Read((void*)weights_L2, sizeof(weights_L2));
}

#endif
