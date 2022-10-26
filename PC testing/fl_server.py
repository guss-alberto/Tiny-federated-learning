import subprocess

import struct
import time
from unittest import result
import numpy as np
#import matplotlib.pyplot as plt
import threading
import time
import json
import os
import random
import speechpy 

NUM_CLIENTS = 3

FFT_WINDOW = 256

MFCC_COEFF =  13
NUM_FRAMES =  49

TRAINING_ROUNDS_BEFORE_FL = 30
NODES_L0 = NUM_FRAMES*MFCC_COEFF
NODES_L1 = 25
NODES_L2 = 3

NN_SIZE = NODES_L1*(NODES_L0+1) + NODES_L2*(NODES_L1+1)


path = "../../TinyML-FederatedLearning-master/"

PROCESS = True

montserrat_files = [file for file in os.listdir(path+"datasets/mountains") if file.startswith("montserrat")]
pedraforca_files = [file for file in os.listdir(path+"datasets/mountains") if file.startswith("pedraforca")]
vermell_files = [file for file in os.listdir(path+"datasets/colors") if file.startswith("vermell")]
verd_files = [file for file in os.listdir(path+"datasets/colors") if file.startswith("verd")]
blau_files = [file for file in os.listdir(path+"datasets/colors") if file.startswith("blau")]
test_montserrat_files = [file for file in os.listdir(path+"datasets/test") if file.startswith("montserrat")]
test_pedraforca_files = [file for file in os.listdir(path+"datasets/test") if file.startswith("pedraforca")]       

def get_training(indextype):
    indextype += 1
    result = []
    data = {}
    for i in range(TRAINING_ROUNDS_BEFORE_FL):
        if indextype == 1:
            data = json.load(open(path+"datasets/mountains/"+montserrat_files.pop()))      
        elif indextype == 2:
            data = json.load(open(path+"datasets/mountains/"+pedraforca_files.pop()))      
        elif indextype == 3:
            data = json.load(open(path+"datasets/colors/"   +vermell_files.pop()))      
        elif indextype == 4:
            data = json.load(open(path+"datasets/colors/"   +verd_files.pop()))      
        elif indextype == 5:
            data = json.load(open(path+"datasets/colors/"   +blau_files.pop()))      
        elif indextype == 6:
            data = json.load(open(path+"datasets/test/"     +test_montserrat_files.pop()))     
            indextype = 1
        elif indextype == 7:
            data = json.load(open(path+"datasets/test/"     +test_pedraforca_files.pop()))  
            indextype = 2
        samples = data["payload"]["values"] 
        data = speechpy.feature.mfcc(np.array(samples), 15999, frame_length = 0.02, frame_stride = 0.02, num_cepstral = 13, num_filters = 32, high_frequency = 0, low_frequency = 300, fft_length = 256, dc_elimination = True).astype('float32')
        result.append(data.tobytes()+struct.pack('b',indextype))
    return result


if __name__ == "__main__":
    devices = [] 
    devices_weights = np.empty((NUM_CLIENTS, NN_SIZE), dtype='float32')
    pathnw = "test/new_weights"
    pathw = "test/weights"
    patht = "test/data"   

    try:
        os.remove(pathw) #reset weights if leftover from previous attempt
        print("Weights reset")  
    except:
        print("File not present")  
    
    while len(montserrat_files)>TRAINING_ROUNDS_BEFORE_FL and len(pedraforca_files)>TRAINING_ROUNDS_BEFORE_FL and len(vermell_files)>TRAINING_ROUNDS_BEFORE_FL:            
        for i in range(NUM_CLIENTS):
            print("Training", i)
            #convert and prepare training data
            with open(patht,"wb") as file:
                file.write(b''.join(get_training(i)))

            #run training
            process = subprocess.run(["./a.exe", patht, pathnw, pathw, "Learn"]) 
            print(process.stdout)
            #get resulting weights from 
            with open(pathw, "rb") as file:
                data = file.read()
                devices_weights[i] = struct.unpack('%sf' % NN_SIZE, data)

        with open(pathnw, "wb") as file: #write updated weights to file
            file.write(b''.join(np.average(devices_weights, axis=0)))

    print("DONE: testing")
    #testing
    testing_data = []
    with open(patht,"wb") as file:
        file.write(b''.join(get_training(5)))
        file.write(b''.join(get_training(2)))

    process = subprocess.run(["./a.exe", patht, pathnw, pathw, "Test"]) 
    print(process.stdout)

    print ("Done")