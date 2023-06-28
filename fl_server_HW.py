
import subprocess
import struct
import time
import numpy as np
#import matplotlib.pyplot as plt
import threading
import time
import json
import os
import random
import speechpy 
import serial
from serial.tools.list_ports import comports

FFT_WINDOW = 512

MFCC_COEFF =  13
NUM_FRAMES =  50
BAUDRATE = 230400

TRAINING_ROUNDS_BEFORE_FL = 30
NODES_L0 = NUM_FRAMES*MFCC_COEFF
NODES_L1 = 21
NODES_L2 = 3

NN_SIZE = NODES_L1*(NODES_L0+1) + NODES_L2*(NODES_L1+1)
N_SAMPLES = 16000
SAMPLE_RATE = 16000
path = "../TinyML-FederatedLearning-master/datasets/"

resfile = "C:/Users/guss/Desktop/datta/Test_20_nodes{}.csv"

PROCESS = True
IID = True  
FILE_SOURCE = "paper"

FRAME_LENGTH_TEST = N_SAMPLES/(SAMPLE_RATE*(NUM_FRAMES+2))


def get_uart_ports():
    client_ports = []
    for port in comports():
        if "UART" in port.description: #find all uart ports
            print(port.name, port.description)
            client_ports.append(port.name)
    return client_ports

device_ports = get_uart_ports()
NUM_CLIENTS = len(device_ports)


TRAINING_SAMPLE = b"\x01"
ML_MODEL =        b"\x02"
MODEL_REQUEST =   b"\x03"


if FILE_SOURCE == "paper":
    montserrat_files = [file for file in os.listdir(path+"mountains") if file.startswith("montserrat")]
    pedraforca_files = [file for file in os.listdir(path+"mountains") if file.startswith("pedraforca")]
    vermell_files = [file for file in os.listdir(path+"colors") if file.startswith("vermell")]
    verd_files = [file for file in os.listdir(path+"colors") if file.startswith("verd")]
    blau_files = [file for file in os.listdir(path+"colors") if file.startswith("blau")]   

    random.shuffle(montserrat_files)
    random.shuffle(pedraforca_files)
    random.shuffle(vermell_files)
    random.shuffle(verd_files)
    random.shuffle(blau_files)

    mountains      = list(sum(zip(montserrat_files, pedraforca_files), ()))
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


def feature_extraction(samples):
    data = speechpy.processing.preemphasis(np.array(samples), shift=1, cof=0.98)
    data = speechpy.feature.mfcc(np.array(samples), SAMPLE_RATE, frame_length = FRAME_LENGTH_TEST, frame_stride = FRAME_LENGTH_TEST, num_cepstral = MFCC_COEFF, num_filters = 32, high_frequency = SAMPLE_RATE/2, low_frequency = 300, fft_length = FFT_WINDOW, dc_elimination = True)
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
            else:
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
            samples = data["payload"]["values"] 
            if PROCESS:
                data = feature_extraction(samples)
                #print(data.shape)
                result.append(struct.pack('b',indextype) + data.tobytes())
            else:
                result.append(struct.pack('b',indextype) + struct.pack(f"{len(samples)}h", *samples))
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
                result.append(struct.pack('b',indextype) + data.tobytes())
            else:
                result.append(data)
    
    return result

def enough_data():
    if FILE_SOURCE == "paper":
        if IID:
            return len(mountains) >= TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS
        else:
            if len(montserrat_files) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(pedraforca_files) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(vermell_files) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            return True
    else:
        if IID:
            return len(mountains) >= NUM_CLIENTS*TRAINING_ROUNDS_BEFORE_FL
        else:
            if len(red_data) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(green_data) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            if len(blue_data) < TRAINING_ROUNDS_BEFORE_FL*NUM_CLIENTS: return False
            return True

def startFL (id, ser, dataset, global_model, local_models, num_epochs):
    rounds = 0
    print("Device %s start"%id)
    if len(global_model)==NN_SIZE:
        print("Writing new data to device %s"%id)
        buf = ML_MODEL + struct.pack('%sf' % NN_SIZE, *global_model) #send new weights back to device
        ser.write(buf)
    for sample in dataset:
        buf = TRAINING_SAMPLE + sample
        ser.write(buf)
        print (buf)
        print("------------")
        tst = ser.read(NODES_L0*4)
        print(tst)
        print(struct.unpack('%sf' % NODES_L0, tst))
        exit()
        rounds+=1
        error_raw = ser.read(4)
        error = struct.unpack("f",error_raw)
        print(error)
    ser.write(MODEL_REQUEST)
    buf = ser.read(2)
    print(buf)
    num_epochs[id] = struct.unpack('h', buf)
    print(num_epochs[id])

    buf = ser.read(NN_SIZE*4)
    local_models[id] = struct.unpack('%sf' % NN_SIZE, buf)

    return

if __name__ == "__main__":    
    client_ports = [] 
    
    for port in device_ports:
        client_ports.append(serial.Serial(port, baudrate=BAUDRATE, bytesize=8, stopbits=serial.STOPBITS_ONE, timeout=10))
    round = 0
    global_model = []
    
    while enough_data():
        local_models = np.empty((len(device_ports), NN_SIZE), dtype='float32')
        threads = []
        num_epochs = []
        print("Round %s"%round)
        
        for i, device in enumerate(client_ports):
            device_dataset = get_training(i+1)
            num_epochs.append(0)
            thread = threading.Thread(target=startFL, args=(i, device, device_dataset, global_model, local_models, num_epochs))
            thread.daemon = True
            thread.start()
            threads.append(thread)
    
        for thread in threads: thread.join() # Wait for all the threads to end

        print ("AVERAGING NN WEIGHTS")
        global_model = np.average(local_models, axis=0, weights=num_epochs).tolist()
        round+=1

    buf = struct.pack('%sf' % NN_SIZE, *global_model) #send new weights back to device
    for device in client_ports:
        device.write(buf)

    print ("Done")

