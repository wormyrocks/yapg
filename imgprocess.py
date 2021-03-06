#!/usr/bin/env/python
from PIL import Image
import struct

infile = raw_input("Enter input file: ");
outfile = raw_input("Enter output file: ");
f = open (outfile, "wb")
im = Image.open(infile)
width = im.size[0]
height = im.size[1]

val = raw_input("Enter speed (1 is fastest): ")
speed = int(val)
header = [height>>8, height & 0xff, width>>8, width & 0xff, speed >> 8, speed & 0xff, 0, 0, 0, 0, 0, 0]

data = header;
framenum = 0

while (im):
    im2 = im.convert("RGB")
    pix = im2.load()
    for j in range (0, width):
        for i in range (0, height):
            data.append(pix[j,i][0])
            data.append(pix[j,i][1])
            data.append(pix[j,i][2])
    framenum += 1
    try: 
        im.seek(framenum)
    except EOFError:
        break

header[6] = framenum >> 8
header[7] = framenum & 0xff

ba = bytearray(data)
f.write(ba)

im.close()
f.close()
