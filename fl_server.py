import serial
from serial.tools.list_ports import comports

import struct
import time
import numpy as np
import matplotlib.pyplot as plt
import threading
import time
import json
import os
import random

MFCC_COEFF =  9
NUM_FRAMES =  28

TRAINING_ROUNDS_BEFORE_FL = 10
NODES_L0 = NUM_FRAMES*MFCC_COEFF
NODES_L1 = 24
NODES_L2 = 3

NN_SIZE = NODES_L1*(NODES_L0+1) + NODES_L2*(NODES_L1+1)

TRAINING_DATASET_FILE = "datasets/colors.dat"
RECODING_DATASET_FILE = "datasets/colors.dat"

BAUDRATE = 230400
device_ports = ['COM4', 'COM12']

FL_READY =          b"\x01"
FL_SEND_DATA =      b"\x02"
REQUEST_TRAINING_DATA =   b"\x03"
RECEIVE_TRAINING =  b"\x05"
CHANGE_MODE =    b"\xff"

MODE_LEARN =        b"\x01"
MODE_EVAL =         b"\x02"
MODE_RECORD =       b"\x03"
DATA_READY =        b"\x04"



def import_dataset (path):
    data = []
    with open(path, "rb") as file:
        while d := file.read(NODES_L0*4+1):
            data.append(d)
    random.shuffle(data)
    return data
    
def startFL (id, ser, dataset, new_weights, devices_weights):
    round = 0
    sent = False
    count = 0
    print("Device %s start"%id)
    if len(new_weights)==NN_SIZE:
        print("Writing new data to device %s"%id)
        buf = struct.pack('%sf' % NN_SIZE, *new_weights) #send new weights back to device
        ser.write(buf)
    mode = MODE_LEARN
    ser.write(b"\xff"+mode) 
    while True:
        if (ser.in_waiting):
            a = ser.read()
            if   ( a == FL_READY ):
                data = ser.read(2)                       #read number of epochs
                [temp] = struct.unpack("H",data)

                data = ser.read(NN_SIZE*4)            #read neural network data
                devices_weights[id] = struct.unpack('%sf' % NN_SIZE, data)
                print("Device %s received weights"%id)
                return
                
            elif ( a == REQUEST_TRAINING_DATA ):
                ser.write(dataset[round])  #send data
                res = ser.read(NODES_L0*4)

                data = ser.read(4)
                [error] = struct.unpack("f",data)
                print("Device %s, round %s, Err: %s"%(id,round,error))
                #errors[id][count]=error
                round += 1
                sent = False

            elif ( a == DATA_READY ):
                data = ser.read(NODES_L0*4+1)
                #print(struct.unpack("%sfc" % NODES_L0, data))
                recording_file.write(data)
                print(id, round)
                round += 1
        elif mode == MODE_LEARN:
            if not sent:
                time.sleep(.1)
                if (count < len(dataset)):
                    ser.write(RECEIVE_TRAINING)
                else:
                    ser.write(FL_READY)
                    print("Device %s start FL"%(id))
                sent = True
                count += 1

if __name__ == "__main__":
    dataset = import_dataset(TRAINING_DATASET_FILE)
    recording_file = open(RECODING_DATASET_FILE, "ab")
    
    devices = [] 
    
    print("%s training samples"%len(dataset))

    for port in device_ports:
        devices.append(serial.Serial(port, baudrate=BAUDRATE, bytesize=8, stopbits=serial.STOPBITS_ONE))
    round = 0
    new_weights = []
    
    while True:
        devices_weights = np.empty((len(device_ports), NN_SIZE), dtype='float32')
        #devices_ERRORS = np.empty((len(device_ports), TRAINING_ROUNDS_BEFORE_FL), dtype='float32')
        threads = []
        print("Round %s"%round)
        
        for i, device in enumerate(devices):
            device_dataset = dataset[TRAINING_ROUNDS_BEFORE_FL*i : TRAINING_ROUNDS_BEFORE_FL*(i+1)]
            thread = threading.Thread(target=startFL, args=(i, device, device_dataset, new_weights, devices_weights))
            thread.daemon = True
            thread.start()
            threads.append(thread)
    
        for thread in threads: thread.join() # Wait for all the threads to end

        print ("AVERAGING NN WEIGHTS")
        new_weights = np.average(devices_weights, axis=0).tolist()
        round+=1
        dataset = dataset[ round * TRAINING_ROUNDS_BEFORE_FL*len(devices) :] # delete already used samples

        if len(dataset)==0:
            break

    buf = struct.pack('%sf' % NN_SIZE, *new_weights) #send new weights back to device
    for device in devices:
        device.write(buf)
        device.write(b"\xff"+MODE_EVAL)

    print ("Done")


        

    
    


