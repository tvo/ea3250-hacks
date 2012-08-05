#!/usr/bin/env python
import serial, lpc32xx_serial

for baud in (9600, 31250):
	ser1 = serial.serial_for_url('/dev/ttyS1', baud)
	ser2 = serial.serial_for_url('/dev/ttyTX0', baud)
	ser1.timeout = 1
	ser2.timeout = 1
	if baud == 31250:
		lpc32xx_serial.set_special_baudrate(ser2, baud)
	print ser1.write('1234567890')
	print ser2.read(10)
	print ser2.write('1234567890')
	print ser1.read(10)
