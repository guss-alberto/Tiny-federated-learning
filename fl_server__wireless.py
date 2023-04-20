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

port = 'COM4'
BAUDRATE = 230400

MFCC_COEFF =  9
NUM_FRAMES =  28

TRAINING_ROUNDS_BEFORE_FL = 10
NODES_L0 = NUM_FRAMES*MFCC_COEFF
NODES_L1 = 24
NODES_L2 = 3

NN_SIZE = NODES_L1*(NODES_L0+1) + NODES_L2*(NODES_L1+1)

TRAINING_DATASET_FILE = "datasets/colors.dat"


CHUNKSIZE = 254

LoRa_serial = serial.Serial(port, baudrate=BAUDRATE, bytesize=8, stopbits=serial.STOPBITS_ONE)

def LoRa_GetPacket(non_blocking = True):
    if non_blocking and not LoRa_serial.in_waiting:
        return None
    size = int.from_bytes(LoRa_serial.read(1))
    if (size > 0):
        return LoRa_serial.read(size) 
    return b""  

def LoRa_SendPacket(data):
    if len(data)>255:
        raise Exception("Maximum packet size is 255 bytes")
    LoRa_serial.write(struct.pack("B",len(data)))
    LoRa_serial.write(data)
    print(LoRa_serial.in_waiting)


TIMEOUT = 0.20 #s
def RDT_send (data):
    sequence = 0
    while sequence*CHUNKSIZE < len(data):
        chunk = struct.pack("B",sequence) + data[sequence*CHUNKSIZE:(sequence+1)*CHUNKSIZE]
        print(f"packet {sequence} sent")
        LoRa_SendPacket (chunk)
        timer = time.time()
        while time.time()-timer < TIMEOUT:
            message = LoRa_GetPacket()
            if message:
                ack = int.from_bytes(message)
                if ack == sequence:
                    sequence += 1
                elif ack < sequence:
                    sequence = ack + 1
                    print("Packet lost")
                break
        else:
            print("Packet Timeout")

def RDT_receive ():
    sequence = 0
    data = b""
    while True:
        chunk = LoRa_GetPacket(non_blocking = False)
        if chunk:
            ack = int.from_bytes(data[0])
            if ack == sequence:
                sequence += 1
                data+=chunk
                LoRa_SendPacket(struct.pack("B",ack))
        else:
            LoRa_SendPacket(struct.pack("B",sequence))

def sendLongMessage (device, dts, message):
    # Send the byte array in chunks
    offset = 0
    while offset < len(message):
        chunk = message[offset:offset+CHUNKSIZE]
        LoRa_SendPacket (chunk)
        offset += CHUNKSIZE
        
def load_ds (path):
    data = []
    with open(path, "rb") as file:
        while d := file.read(N_SAMPLES*2+1):
            data.append(d)
    return data



if __name__ == "__main__":
    lorem_ipsum = r"""auctor tortor aliquet at. Aenean arcu metus, dapibus eu leo ut, placerat porttitor sem. Donec fermentum felis vel dictum dapibus. Etiam eleifend sodales ante, nec maximus leo suscipit vitae. Proin tincidunt lacinia vulputate. Proin sed ligula a mi auctor porta et at velit. Pellentesque ac pulvinar metus. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Ut porttitor eget neque ac fringilla. Phasellus malesuada venenatis nisi, at elementum urna dignissim vitae. Maecenas iaculis augue tortor, at venenatis nisl suscipit vel. Curabitur sit amet nisl in libero elementum sagittis a sit amet enim. Donec at elit vulputate, sodales nisl et, laoreet dui. Donec id quam quis tortor tincidunt laoreet quis sit amet massa.

Nunc nec turpis sit amet tellus iaculis fringilla ut non arcu. Nunc ultricies non orci et ultricies. Quisque laoreet accumsan lorem vel tincidunt. Duis a sapien ac turpis pellentesque rutrum sed sit amet sem. Maecenas auctor turpis sit amet feugiat faucibus. Curabitur eu neque massa. Vivamus euismod hendrerit posuere. Vestibulum semper accumsan velit, nec fringilla ipsum auctor vel. Vivamus ultrices leo et tristique tempus. Proin id metus dui. Integer non dolor cursus, sagittis velit vitae, semper nibh. Duis suscipit neque non purus tempus, eget consectetur nunc dictum.

Curabitur dolor risus, pretium ac nunc eget, malesuada eleifend enim. Aenean eu sagittis mauris, finibus gravida nibh. Ut viverra fusce."""
    
    message = lorem_ipsum.encode('utf-8')
    RDT_send (message)
    #dataset = load_ds()
    