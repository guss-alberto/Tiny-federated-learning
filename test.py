from socket import timeout
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
MODE_EVAL =         b"\x02"
MODE_RECORD =       b"\x03"
DATA_READY =        b"\x04"
RECEIVE_TRAINING =  b"\x05"

i = 0

if __name__ == "__main__":
    ser = serial.Serial(PORT, baudrate=BAUDRATE, bytesize=8, timeout=10, stopbits=serial.STOPBITS_ONE)
    ser.write(b"\xff"+MODE_RECORD)  #set mode to training
    ser.read()
    while True:
        if (ser.in_waiting):
            a = ser.read(4)
            [num] = struct.unpack("f",a)
            i+=1
            print(num)
            print(i)

