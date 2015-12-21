# yapg
yet another POV globe

https://hackaday.io/project/8881-yapg

uses custom Adafruit DotStar library to drive a blindingly bright LED string
and an FPGA control loop to handle the motor control

compile yapg.c to run the motor control loop

compile write_file.c with -lncurses to run the LED editor

compile and upload code in verilog_code folder to run the motor control loop
(map pins at your own risk)

run imgprocess.py to convert an image into an LED file
for best results the image should be as tall as the LED strip
