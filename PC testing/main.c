#include "includes.h"
//#include "feature_extraction.h"
#include "ml.h"

void simple_train(uint8_t class);


union { //use union to save memory
    int16_t rec[NUM_SAMPLES];
    struct {
        float input[NODES_L0];
        float out[NODES_L2];
        float target[NODES_L2];
        float error;
    } ml;
} myData;

int main(void)
{
    srand(RANDOM_SEED);
    FILE* recfile =  fopen("../datasets/mfcc_mountains.dat","rb");
    if (!recfile){
        fprintf(stderr, "ERROR, Couldn't open file");
        return 1;
    }
    uint8_t a;
    uint32_t i;

    ml_init();
    while (!feof(recfile)){
        #ifndef READ_PROCESSED
            fread(myData.rec, FFT_WINDOW*NUM_FRAMES, 2, recfile); //read recording
            feature_extraction(myData.rec, myData.ml.input);
        #else
            fread(myData.ml.input, NODES_L0, 4, recfile); //read recording
        #endif
        fread(&a, 1, 1, recfile); //read class
        simple_train(a);
    }
}


void simple_train(uint8_t class){
    myData.ml.target[0] = 0;
    myData.ml.target[1] = 0;
    myData.ml.target[2] = 0;
    myData.ml.target[class-1] = 1.0; // button 1 -> {1,0,0};  button 2 -> {0,1,0};  button 3 -> {0,0,1}

    myData.ml.error = learn (myData.ml.input, myData.ml.out, myData.ml.target); //train

    printf("%.2f | %.2f | %.2f, error: %.5f, target %d, %d\n", myData.ml.out[0], myData.ml.out[1], myData.ml.out[2], myData.ml.error, class, num_epochs);

    num_epochs++;
}






