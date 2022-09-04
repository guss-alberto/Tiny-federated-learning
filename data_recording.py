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


FFT_WINDOW =  256
NUM_FRAMES = 28

NUM_SAMPLES = FFT_WINDOW*NUM_FRAMES

NODES_L0 = NUM_FRAMES*MFCC_COEFF

RECODING_DATASET_FILE = "datasets/colors.dat"
RAW_RECODING_DATASET_FILE = "datasets/colors.dat"

BAUDRATE = 230400

FL_READY =          b"\x01"
FL_SEND_DATA =      b"\x02"
REQUEST_TRAINING_DATA =   b"\x03"
RECEIVE_TRAINING =  b"\x05"
CHANGE_MODE =       b"\xff"

MODE_LEARN =        b"\x01"
MODE_EVAL =         b"\x02"
MODE_RECORD =       b"\x03"
DATA_READY =        b"\x04"
RAW_DATA_READY =    b"\x06"

if __name__ == "__main__":
    recording_file = open(RECODING_DATASET_FILE, "ab")
    raw_recording_file = open(RAW_RECODING_DATASET_FILE, "ab")
    
    ser = serial.Serial("COM4", baudrate=BAUDRATE, bytesize=8, stopbits=serial.STOPBITS_ONE)
    round = 0
    count = 0
    mode = MODE_RECORD
    ser.write(b"\xff"+mode) 
    while True:
        if (ser.in_waiting):
            a = ser.read()
            
            if ( a == DATA_READY ):
                data = ser.read(NODES_L0*4+1) #data is 32 bit float + 1 byte for class
                recording_file.write(data) 
                print(round)
                round += 1
            elif (a == RAW_DATA_READY):
                data = ser.read(NUM_SAMPLES*2+1) #data is 16 bit + 1 byte for class
                raw_recording_file.write(data)
                print(round)
                round += 1
        time.sleep(0.1)
    


        

    
    


