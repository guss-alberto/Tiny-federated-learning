from concurrent.futures import process
import serial
import struct

import numpy as np
import json
import os
import random
from python_speech_features import mfcc
import speechpy 

path = "../../TinyML-FederatedLearning-master/"

PROCESS = True

montserrat_files = [file for file in os.listdir(path+"datasets/mountains") if file.startswith("montserrat")]
pedraforca_files = [file for file in os.listdir(path+"datasets/mountains") if file.startswith("pedraforca")]
vermell_files = [file for file in os.listdir(path+"datasets/colors") if file.startswith("vermell")]
verd_files = [file for file in os.listdir(path+"datasets/colors") if file.startswith("verd")]
blau_files = [file for file in os.listdir(path+"datasets/colors") if file.startswith("blau")]
test_montserrat_files = [file for file in os.listdir(path+"datasets/test/") if file.startswith("montserrat")]
test_pedraforca_files = [file for file in os.listdir(path+"datasets/test") if file.startswith("pedraforca")]


result = []


for row in zip(montserrat_files, pedraforca_files, vermell_files):
    for i, f in enumerate(row):
        try:
            data = json.load(open(path+"datasets/colors/"+f))
        except:
            data = json.load(open(path+"datasets/mountains/"+f))
        samples = data["payload"]["values"]
        if not PROCESS:
            result.append(struct.pack('%sh' % len(samples), *samples)+struct.pack('b',i+1))
        else:
            #data = mfcc(np.array(samples), 16000, winlen=0.02, winstep = 0.02, numcep=13, nfilt=32, highfreq = 0, lowfreq = 300, nfft=320, ceplifter = 0, preemph = .98).astype('float32')
            data = speechpy.feature.mfcc(np.array(samples), 15999, frame_length = 0.02, frame_stride = 0.02, num_cepstral = 13, num_filters = 32, high_frequency = 0, low_frequency = 300, fft_length = 256, dc_elimination = True).astype('float32')
            print(data.shape)
            result.append(data.tobytes()+struct.pack('b',i+1))


with open("mfcc_mountains.dat", "wb") as file:
    file.write(b''.join(result))