#!/usr/bin/env/python
from PIL import Image
import struct

infile = raw_input("Enter input file: ");
outfile = raw_input("Enter output file: ");
f = open (outfile, "wb")
im = Image.open(infile)
im = im.convert("RGB")
width = im.size[0]
height = im.size[1]

# f.write(struct.pack("@i", height))
# f.write(struct.pack("@i", width))
# f.write(struct.pack("@i", 1))
# f.write(struct.pack("@i", 1))

header = [height, width, 1, 1, 0, 0, 0, 0]

data = header;
framenum = 0

while (im):
    pix = im.load()
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

header[2] = framenum

ba = bytearray(data)
f.write(ba)

im.close()
f.close()
