#include "includes.h"
#include "feature_extraction.h"
#include "ml.h"

void simple_train(uint8_t class, bool mode);


union { //use union to save memory
    int16_t rec[NUM_SAMPLES];
    struct {
        float input[NODES_L0];  
        float out[NODES_L2];
        float target[NODES_L2];
        float error;
    } ml;
} myData;

int main(int argc, char **argv){
    srand(RANDOM_SEED);

    if (argc!=5){
        fprintf(stderr, "ERROR, Invalid number of parameters%d",argc);
        return 2;
    }


    FILE* recfile =  fopen(argv[1],"rb");
    if (!recfile){
        fprintf(stderr, "ERROR, Couldn't open file");
        return 1;
    }

    FILE* weightsfile =  fopen(argv[2],"rb");
    if (weightsfile){
        printf("Weights loaded\n");
        getModel(weightsfile);
        fclose(weightsfile);
    } else {
        ml_init();
    }

    /*FILE* dirfile =  fopen(argv[5],"rb");
    if (dirfile){
        printf("Direction loaded\n");
        getDir(dirfile);
        fclose(dirfile);
    }*/

    bool mode = (argv[4][0]=='T');
    if (mode){
        printf("Testing mode\n");
    }
    uint8_t a;

    while (!feof(recfile)){
        #ifndef READ_PROCESSED
            fread(myData.rec, FFT_WINDOW*NUM_FRAMES, 2, recfile); //read recording
            feature_extraction(myData.rec, myData.ml.input);
        #else
            fread(myData.ml.input, NODES_L0, 4, recfile); //read recording
        #endif
        fread(&a, 1, 1, recfile); //read classi
        simple_train(a, mode);

    }

    weightsfile =  fopen(argv[3],"wb");
    if (!weightsfile){
        fprintf(stderr, "ERROR, Couldn't open file");
        return 1;
    }
    sendModel(weightsfile);
    /*dirfile =  fopen(argv[5],"wb");
    if (!dirfile){
        fprintf(stderr, "ERROR, Couldn't open file");
        return 1;
    }
    sendDir(dirfile);*/
}


void simple_train(uint8_t class, bool mode){
    myData.ml.target[0] = 0;
    myData.ml.target[1] = 0;
    myData.ml.target[2] = 0;
    myData.ml.target[class-1] = 1.0; // button 1 -> {1,0,0};  button 2 -> {0,1,0};  button 3 -> {0,0,1}
    if (!mode)
        myData.ml.error = learn (myData.ml.input, myData.ml.out, myData.ml.target); //train
    else
        myData.ml.error = eval (myData.ml.input, myData.ml.out, myData.ml.target); //train
    
    printf("%.2f, %.2f, %.2f, %.5f, %d, %d\n", myData.ml.out[0], myData.ml.out[1], myData.ml.out[2], myData.ml.error, class, num_epochs);

    num_epochs++;
}


