#!/bin/bash
# Author: Tobi Vollebregt

while true; do
	x=`cat /sys/devices/platform/lpc32xx-adc/adin0`
	y=`cat /sys/devices/platform/lpc32xx-adc/adin1`
	z=`cat /sys/devices/platform/lpc32xx-adc/adin2`
	echo -n "values : $x $y $z  " $'\r'
	sleep 0.1
done
