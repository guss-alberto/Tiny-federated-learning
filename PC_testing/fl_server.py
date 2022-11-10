
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
NUM_FRAMES =  50

TRAINING_ROUNDS_BEFORE_FL = 30
NODES_L0 = (NUM_FRAMES)*MFCC_COEFF
NODES_L1 = 25
NODES_L2 = 3

NN_SIZE = NODES_L1*(NODES_L0+1) + NODES_L2*(NODES_L1+1)
N_SAMPLES = 16000
SAMPLE_RATE = 16000
path = "../../TinyML-FederatedLearning-master/datasets/"

PROCESS = False
IID = True 
TESTING = True
FILE_SOURCE = "paper"

FRAME_LENGTH_TEST = N_SAMPLES/(SAMPLE_RATE*NUM_FRAMES)


if FILE_SOURCE == "paper":
    montserrat_files = [file for file in os.listdir(path+"mountains") if file.startswith("montserrat")]
    pedraforca_files = [file for file in os.listdir(path+"mountains") if file.startswith("pedraforca")]
    vermell_files = [file for file in os.listdir(path+"colors") if file.startswith("vermell")]
    verd_files = [file for file in os.listdir(path+"colors") if file.startswith("verd")]
    blau_files = [file for file in os.listdir(path+"colors") if file.startswith("blau")]
    test_montserrat_files = [file for file in os.listdir(path+"test") if file.startswith("montserrat")]
    test_pedraforca_files = [file for file in os.listdir(path+"test") if file.startswith("pedraforca")]       

    mountains      = list(sum(zip(montserrat_files, pedraforca_files), ()))
    test_mountains = list(sum(zip(test_montserrat_files, test_pedraforca_files), ()))
else :
    def load_ds (path):
        data = []
        with open(path, "rb") as file:
            while d := file.read(N_SAMPLES*2+1):
                data.append(d)
        return data

    red_data   = load_ds("../datasets/raw_colors_red.dat",)
    green_data = load_ds("../datasets/raw_colors_green.dat")
    blue_data  = load_ds("../datasets/raw_colors_blue.dat")

    colors     = list(sum(zip(red_data, green_data, blue_data), ()))


def feature_extraction(samples):
    data = speechpy.processing.preemphasis(np.array(samples), shift=1, cof=0.98)
    data = speechpy.feature.mfcc(data, SAMPLE_RATE, frame_length = FRAME_LENGTH_TEST, frame_stride = FRAME_LENGTH_TEST, num_cepstral = MFCC_COEFF, num_filters = 32, high_frequency = 0, low_frequency = 300, fft_length = 512, dc_elimination = True)
    data = speechpy.processing.cmvnw(data, win_size=101, variance_normalization=True)
    return data.astype('float32')

def get_training(indextype):
    result = []
    data = {}
    if FILE_SOURCE == "paper":
        for i in range(TRAINING_ROUNDS_BEFORE_FL):
            if IID:
                filename = mountains.pop()
                if (filename.startswith("montserrat")):
                    indextype = 1
                elif (filename.startswith("pedraforca")):
                    indextype = 2
                data = json.load(open(path+"mountains/"+filename))
            try:
                if indextype == 1:
                    data = json.load(open(path+"mountains/"+montserrat_files.pop()))      
                elif indextype == 2:
                    data = json.load(open(path+"mountains/"+pedraforca_files.pop()))      
                elif indextype == 3:
                    data = json.load(open(path+"colors/"   +vermell_files.pop()))      
                elif indextype == 4:
                    data = json.load(open(path+"colors/"   +verd_files.pop()))      
                elif indextype == 5:
                    data = json.load(open(path+"colors/"   +blau_files.pop()))      
                elif indextype == 6:
                    data = json.load(open(path+"test/"     +test_montserrat_files.pop()))     
                    indextype = 1
                elif indextype == 7:
                    data = json.load(open(path+"test/"     +test_pedraforca_files.pop()))  
                    indextype = 2
            except Exception as e:
                print (e)
                continue
            samples = data["payload"]["values"] 
            if PROCESS:
                data = feature_extraction(samples)
                result.append(data.tobytes()+struct.pack('b',indextype))
            else:
                result.append(struct.pack('%sh' % len(samples), *samples)+struct.pack('b',indextype))
    else:
        for i in range(TRAINING_ROUNDS_BEFORE_FL):
            if IID:
                data = colors.pop()
            else:
                if indextype == 1:
                    data =  red_data.pop()
                elif indextype == 2:
                    data = green_data.pop()
                elif indextype == 3:
                    data =  blue_data.pop()     
            if PROCESS:
                indextype = data[-1]
                data = feature_extraction(data[0:-1])
                result.append(data.tobytes()+struct.pack('b',indextype))
            else:
                result.append(data)
    
    return result

def enough_data():
    if FILE_SOURCE == "paper":
        if IID:
            return len(mountains) >= NUM_CLIENTS*TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS
        else:
            if len(montserrat_files) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(pedraforca_files) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(vermell_files) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            return True
    else:
        if IID:
            return len(mountains) >= NUM_CLIENTS*TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS
        else:
            if len(red_data) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(green_data) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(blue_data) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            return True
def get_testing ():
    result = []
    data = {}
    while len(test_mountains)>0:
        filename = test_mountains.pop()
        if (filename.startswith("montserrat")):
            indextype = 1
        elif (filename.startswith("pedraforca")):
            indextype = 2
        data = json.load(open(path+"test/"+filename))
        
        samples = data["payload"]["values"] 
        if PROCESS:
            data = speechpy.feature.mfcc(np.array(samples), 15999, frame_length = 0.02, frame_stride = 0.02, num_cepstral = 13, num_filters = 32, high_frequency = 0, low_frequency = 300, fft_length = 256, dc_elimination = True).astype('float32')
            result.append(data.tobytes()+struct.pack('b',indextype))
        else:
            result.append(struct.pack('%sh' % len(samples), *samples)+struct.pack('b',indextype))
    return result

if __name__ == "__main__":
    devices = [] 
    devices_weights = np.empty((NUM_CLIENTS, NN_SIZE), dtype='float32')
    pathnw = "test/new_weights"
    pathw = "test/weights"
    patht = "test/data"   

    try:
        os.remove(pathnw) #reset weights if leftover from previous attempt
        print("Weights reset")  
    except:
        print("File not present")  
    
    while enough_data():            
        for i in range(NUM_CLIENTS):
            print("Training", i)
            #convert and prepare training data
            with open(patht,"wb") as file:
                file.write(b''.join(get_training(i+1)))

            #run training
            process = subprocess.run(["./a.exe", patht, pathnw, pathw, "Learn"]) 
            print(process.stdout)
            #get resulting weights from 
            with open(pathw, "rb") as file:
                data = file.read()
                devices_weights[i] = struct.unpack('%sf' % NN_SIZE, data)

        with open(pathnw, "wb") as file: #write updated weights to file
            file.write(b''.join(np.average(devices_weights, axis=0)))

    with open(pathnw, "wb") as file: #write updated weights to file
            file.write(b''.join(np.average(devices_weights, axis=0)))

    print("DONE: testing")
    #testing
    if TESTING:
        with open(patht,"wb") as file:
            data = get_testing()
            #random.shuffle(data)
            file.write(b''.join(data))

        process = subprocess.run(["./a.exe", patht, pathnw, pathw, "Test"]) 
        print(process.stdout)

    print ("Done")