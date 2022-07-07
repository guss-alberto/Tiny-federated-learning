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

ser = serial.Serial('COM4', baudrate=115200, bytesize=8, timeout=1, stopbits=serial.STOPBITS_ONE)

file = open('file.bin', 'wb')

sample = []
for i in range(16000):
    data = ser.read(2)
    #sample.append(struct.unpack('h', data))
    file.write(data)

file2 = open('processed.bin', 'w')

sample = []
for i in range(20*11):
    data = ser.read(4)
    [float_num] = struct.unpack('f', data)
    print(float_num)
