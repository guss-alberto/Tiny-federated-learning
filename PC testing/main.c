#include "includes.h"
#include "fft.h"
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
    init();
    FILE* recfile =  fopen("../datasets/raw_colors.dat","rb")
    if (!recfile){
        fprintf(stderr, "ERROR, Couldn't open file");
        return 1;
    }
    uint8_t a;
    uint32_t i;
    ml_init();
    while (!feof(recfile)){
        fread(recfile,myData.rec,FFT_WINDOW*NUM_FRAMES); //read recording
        fread(recfile,&a,1); //read class
        feature_extraction(myData.rec, myData.ml.input);
        simple_train(a);
        break;
    }
}


void simple_train(uint8_t class){
    myData.ml.target[0] = 0;
    myData.ml.target[1] = 0;
    myData.ml.target[2] = 0;
    myData.ml.target[class-1] = 1.0; // button 1 -> {1,0,0};  button 2 -> {0,1,0};  button 3 -> {0,0,1}

    myData.ml.error = learn (myData.ml.input, myData.ml.out, myData.ml.target); //train

    char str[30];
    sprintf(str, "%.2f | %.2f | %.2f", myData.ml.out[0], myData.ml.out[1], myData.ml.out[2]);

    Graphics_drawString(&ctx, "DONE!........", 20, 10, 20, true);
    Graphics_drawString(&ctx, (int8_t*)str, 30, 10, 30, true);
    sprintf(str, "ERR: %.2f", myData.ml.error);
    Graphics_drawString(&ctx, (int8_t*)str, 30, 10, 40, true);

    num_epochs++;
}






