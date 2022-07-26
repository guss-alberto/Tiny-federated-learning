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
PORT = 'COM4'

FL_READY =          b"\x01"
FL_SEND_DATA =      b"\x02"
REQUEST_TRAINING_DATA =   b"\x03"
CHANGE_MODE =    b"\xff"

MODE_LEARN =        b"\x01"
MODE_EVAL =         b"\x02"
MODE_RECORD =       b"\x03"
DATA_READY =        b"\x04"
RECEIVE_TRAINING =  b"\x05"



nn_weights = []
num_epoch = 0
sent = False
num_dataset = 0

def import_dataset (path):
    data = []
    with open(path, "rb") as file:
        while d := file.read(NODES_L0*4+1):
            data.append(d)
    random.shuffle(data)
    return data
    

if __name__ == "__main__":
    ser = serial.Serial(PORT, baudrate=BAUDRATE, bytesize=8, stopbits=serial.STOPBITS_ONE)
    mode = MODE_LEARN
    ser.write(b"\xff"+mode) 

    colors_dataset = import_dataset(TRAINING_DATASET_FILE)
    recording_file = open(RECODING_DATASET_FILE, "ab")
    while True:
        if (ser.in_waiting):
            a = ser.read()
            if   ( a == FL_READY ):
                data = ser.read(2)                       #read number of epochs
                [temp] = struct.unpack("H",data)
                num_epoch += temp
               
                data = ser.read(NN_SIZE*4)            #read neural network data
                nn_weights = struct.unpack('%sf' % NN_SIZE, data)
                ########### DO STUFF ###########
                buf = struct.pack('%sf' % NN_SIZE, *nn_weights) #send new weights back to device
                ser.write(buf)
            elif ( a == REQUEST_TRAINING_DATA ):
                ser.write(colors_dataset[num_dataset])  #send data
                res = ser.read(NODES_L0*4)

                data = ser.read(4)
                [temp] = struct.unpack("f",data)
                num_dataset += 1
                print(num_dataset)
                print(temp) #read error
                sent = False

            elif ( a == DATA_READY ):
                data = ser.read(NODES_L0*4+1)
                #print(struct.unpack("%sfc" % NODES_L0, data))
                recording_file.write(data)
                print(num_dataset)
                num_dataset += 1
        elif mode == MODE_LEARN:
            if num_dataset<len(colors_dataset) and not sent:
                time.sleep(.1)
                ser.write(RECEIVE_TRAINING)
                sent = True
            elif (num_dataset == len(colors_dataset)):
                print("done")
                mode=MODE_EVAL
                ser.write(b"\xff"+mode)
                num_dataset+=1


