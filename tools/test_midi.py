#!/usr/bin/env python
# Author: Tobi Vollebregt

import serial, time

ser = serial.serial_for_url('/dev/ttyS1', 31250)

try:
	while True:
		print ser.write('\x90\x40\x7f')
		time.sleep(3)
except KeyboardInterrupt:
	pass
