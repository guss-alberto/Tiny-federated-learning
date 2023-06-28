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

    
    if (argc!=
        #ifdef USE_MOMENTUM
            6
        #else 
            5
        #endif
        ){
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
        //printf("Weights loaded\n");
        getModel(weightsfile);
        fclose(weightsfile);
    } else {
        ml_init();
    }
    
    #ifdef USE_MOMENTUM
    FILE* dirfile =  fopen(argv[5],"rb");
        if (dirfile){
            //printf("Momentum loaded\n");
            getDir(dirfile);
            fclose(dirfile);
        }
    #endif

    bool mode = (argv[4][0]=='T');
    if (mode){
        //printf("Testing mode\n");
    }

    uint8_t a;
    srand(RANDOM_SEED);
    init_mfcc();
    while (!feof(recfile)){
        #ifndef READ_PROCESSED
            fread(myData.rec, NUM_SAMPLES, 2, recfile); //read recording
            feature_extraction(myData.rec, myData.ml.input);
            
            /*for (int i = 0; i < (NUM_FRAMES); i++){
                for (int j = 0; j < (MFCC_COEFF); j++)
                    printf("%f\t",myData.ml.input[i*MFCC_COEFF+j] );
                putchar('\n');
            }
            return 1;*/
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

    #ifdef USE_MOMENTUM
        dirfile =  fopen(argv[5],"wb");
        if (!dirfile){
            fprintf(stderr, "ERROR, Couldn't open file");
            return 1;
        }
        sendDir(dirfile);
    #endif
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


