#!/usr/bin/env/python
from PIL import Image
infile = raw_input("Enter input file: ");
outfile = raw_input("Enter output file: ");
f = open (outfile, "wb")
im = Image.open(infile)
pix = im.load()
width = im.size[0]
height = im.size[1]

header = [60, 121, 1, 1, 0, 0, 0, 0]

data = header;

for j in range (0, width):
    for i in range (0, height):
        data.append(pix[j,i][0])
        data.append(pix[j,i][1])
        data.append(pix[j,i][2])

ba = bytearray(data)
f.write(ba)

im.close()
f.close()