import random

data = []
with open("raw_colors_red.dat", "rb") as file:
    while d := file.read(256*30*2+1):
        data.append(d)
with open("raw_colors_green.dat", "rb") as file:
    while d := file.read(256*30*2+1):
        data.append(d)
#with open("raw_colors_blue.dat", "rb") as file:
#    while d := file.read(256*30*2+1):
#        data.append(d)
random.shuffle(data)

with open("raw_color_rando.dat", "wb") as file:
    file.write(b''.join(data))