#!/bin/bash
# Author: Tobi Vollebregt

renice -n 19 $$

LED1=/sys/class/leds/eabb\:red\:led1
LED2=/sys/class/leds/eabb\:red\:led2
LED3=/sys/class/leds/eabb\:red\:led3
LED4=/sys/class/leds/eabb\:red\:led4
LED5=/sys/class/leds/eabb\:red\:led5
LED6=/sys/class/leds/eabb\:red\:led6
LED7=/sys/class/leds/eabb\:red\:led7
LED8=/sys/class/leds/eabb\:red\:led8

LEDS="$LED4 $LED3 $LED2 $LED1 $LED5 $LED6 $LED7 $LED8"

while true; do
	for i in $LEDS; do echo 255 > $i/brightness; sleep 0.1; done
	for i in $LEDS; do echo 0 > $i/brightness; sleep 0.1; done
done
