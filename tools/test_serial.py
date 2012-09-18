#!/usr/bin/env python
# Author: Tobi Vollebregt

import serial, sys

n = len(sys.argv)
device = sys.argv[1] if n > 1 else '/dev/ttyS1'
baud = int(sys.argv[2]) if n > 2 else 9600
ser = serial.serial_for_url(device, baud)
print ser.write('1234567890')
print ser.read(10)
