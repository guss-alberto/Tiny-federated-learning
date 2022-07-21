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


BAUDRATE = 230400
PORT = 'COM4'

FL_READY =          b"\x01"
FL_SEND_DATA =      b"\x02"
FL_REQUEST_DATA =   b"\x03"
FL_CHANGE_MODE =    b"\xff"

MODE_LEARN =        b"\x01"
MODE_LEARN_REMOTE = b"\x05"
MODE_EVAL =         b"\x02"
MODE_RECORD =       b"\x03"
DATA_READY =        b"\x04"


nn_weights = []
num_epoch = 0

num_dataset = 0

def import_dataset (path):
    f = open(path, "rb")
    data = []
    while d := f.read(NODES_L0*4+1):
        data.append(d)
    random.shuffle(data)
    return data

if __name__ == "__main__":
    ser = serial.Serial(PORT, baudrate=BAUDRATE, bytesize=8, stopbits=serial.STOPBITS_ONE)
    ser.write(b"\xff"+MODE_LEARN_REMOTE)  #set mode to training
    #f = open("datasets/colors.dat", "ab")
    colors_dataset = import_dataset("datasets/colors.dat")
    while True:
        if (ser.in_waiting):
            a = ser.read()
            """if   ( a == FL_READY ):
                data = ser.read(2)                       #read number of epochs
                [temp] = struct.unpack("H",data)
                num_epoch += temp
               
                data = ser.read(NN_SIZE*4)            #read neural network data
                nn_weights = struct.unpack('%sf' % NN_SIZE, data)
                ########### DO STUFF ###########
                buf = struct.pack('%sf' % NN_SIZE, *nn_weights) #send new weights back to device
                ser.write(buf)"""
            if ( a == FL_REQUEST_DATA ):
                
                
                if (num_dataset >= len(colors_dataset)):
                    print("train")
                    ser.write(b"\xff\xff"+MODE_EVAL)
                    break
                else:
                    ser.write(b"\x00") 
                    ser.write(colors_dataset[num_dataset])  #send data
                    num_dataset += 1
                    
                    data = ser.read(4)
                    [temp] = struct.unpack("f",data)
                    print(temp)

            """if ( a == DATA_READY ):
                data = ser.read(NODES_L0*4+1)
                f.write(data)
                print(num_dataset)
                num_dataset += 1"""
                


