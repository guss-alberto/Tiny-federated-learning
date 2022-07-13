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

NUM_MEL_BANDS = 22
MFCC_COEFF =  9
NUM_FRAMES =  32
TRAINING_ROUNDS_BEFORE_FL = 10
NODES_L1 = 25
NODES_L2 = 3
BAUDRATE = 115200
devices = ["COM4"]

floatlist = [1.0022, 2.244564, 1.963e27]

com = []
for i in devices:
    com.append(serial.Serial(i, baudrate=BAUDRATE, bytesize=8, timeout=1, stopbits=serial.STOPBITS_ONE))

file = open('file.bin', 'wb')


def send_network (device):
    buf = struct.pack('%sf' % len(floatlist), *floatlist)
    print(device.write(buf))
    print(buf)


if __name__ == "__main__":
    send_network(com[0])
    #while True:

