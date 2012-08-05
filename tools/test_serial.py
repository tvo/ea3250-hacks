#!/usr/bin/env python
import serial, sys
ser = serial.serial_for_url(sys.argv[1])
print ser.write('1234567890')
print ser.read(10)
