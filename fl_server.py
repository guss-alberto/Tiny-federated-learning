import subprocess
import struct
import time
from unittest import result
import numpy as np
from serial.tools.list_ports import comports
#import matplotlib.pyplot as plt
import threading
import time
import json
import os
import random
import speechpy 

NUM_CLIENTS = 3

FFT_WINDOW = 512

MFCC_COEFF =  13
NUM_FRAMES =  50

ROUNDS_BEFORE_FL = 30
NODES_L0 = NUM_FRAMES*MFCC_COEFF
NODES_L1 = 25
NODES_L2 = 3

NN_SIZE = NODES_L1*(NODES_L0+1) + NODES_L2*(NODES_L1+1)
N_SAMPLES = 16000
SAMPLE_RATE = 16000
path = "../TinyML-FederatedLearning-master/datasets/"

PROCESS = True
IID = True 
TESTING = True
FILE_SOURCE = "paper"

BAUDRATE = 230400
device_ports = get_uart_ports()

TRAINING_SAMPLE = b"\x01"
ML_MODEL =        b"\x02"
MODEL_REQUEST =   b"\x03"

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
else:
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


def get_uart_ports():
    client_ports = []
    for port in comports():
        if "UART" in port.description: #find all uart ports
            print(port.name, port.description)
            client_ports.append(port.name)
    return client_ports

    
def startFL (id, ser, dataset, global_model, local_models):
    rounds = 0
    print("Device %s start"%id)
    if len(global_model)==NN_SIZE:
        print("Writing new data to device %s"%id)
        buf = ML_MODEL + struct.pack('%sf' % NN_SIZE, *global_model) #send new weights back to device
        ser.write(buf)
    for i in range(ROUNDS_BEFORE_FL):
        sample = dataset[round]
        buf = TRAINING_SAMPLE + struct.pack(f"{len(sample)}f",sample)
        ser.write(buf)
        rounds+=1
        error_raw = ser.read(4)
        error = struct.unpack('f', error_raw)
        print(f"Error: {error}")
    ser.write(MODEL_REQUEST)
    buf = ser.read(NN_SIZE*4)
    local_models[id] = struct.unpack('%sf' % NN_SIZE, buf)

    return
                

def feature_extraction(samples):
    data = speechpy.processing.preemphasis(np.array(samples), shift=1, cof=0.98)
    data = speechpy.feature.mfcc(np.array(samples), SAMPLE_RATE, frame_length = FRAME_LENGTH_TEST, frame_stride = FRAME_LENGTH_TEST, num_cepstral = MFCC_COEFF, num_filters = 32, high_frequency = SAMPLE_RATE/2, low_frequency = 300, fft_length = FFT_WINDOW, dc_elimination = True)
    data = speechpy.processing.cmvnw(data, win_size=101, variance_normalization=True)
    #for i in range(NUM_FRAMES):
    #    line = ""
    #    for j in range(MFCC_COEFF):
    #        line += f"{data[i][j]:.6f}\t"
    #    print(line)
    #exit()
    return data.astype('float32')

def get_training(indextype):
    result = []
    data = {}
    if FILE_SOURCE == "paper":
        for i in range(ROUNDS_BEFORE_FL):
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
                #print(data.shape)
                result.append(data.tobytes()+struct.pack('b',indextype))
            else:
                result.append(struct.pack(f"{len(samples)}h", *samples)+struct.pack('b',indextype))
    else:
        for i in range(ROUNDS_BEFORE_FL):
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

if __name__ == "__main__":
    dataset = import_dataset(TRAINING_DATASET_FILE)
    
    client_ports = [] 
    
    print("%s training samples"%len(dataset))

    for port in device_ports:
        client_ports.append(serial.Serial(port, baudrate=BAUDRATE, bytesize=8, stopbits=serial.STOPBITS_ONE))
    round = 0
    global_model = []
    
    while True:
        local_models = np.empty((len(device_ports), NN_SIZE), dtype='float32')
        threads = []
        print("Round %s"%round)
        
        for i, device in enumerate(client_ports):
            device_dataset = dataset[ROUNDS_BEFORE_FL*i : ROUNDS_BEFORE_FL*(i+1)]
            thread = threading.Thread(target=startFL, args=(i, device, device_dataset, global_model, local_models))
            thread.daemon = True
            thread.start()
            threads.append(thread)
    
        for thread in threads: thread.join() # Wait for all the threads to end

        print ("AVERAGING NN WEIGHTS")
        global_model = np.average(local_models, axis=0).tolist()
        round+=1
        dataset = dataset[ round * ROUNDS_BEFORE_FL*len(client_ports) :] # delete already used samples

        if len(dataset)==0:
            break

    buf = struct.pack('%sf' % NN_SIZE, *global_model) #send new weights back to device
    for device in client_ports:
        device.write(buf)

    print ("Done")


