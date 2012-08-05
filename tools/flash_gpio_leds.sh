#!/bin/bash
# Author: Tobi Vollebregt

GPIO=/sys/class/gpio
LED3=$GPIO/p2.10
LED4=$GPIO/p2.11
LED5=$GPIO/p2.12

echo 42 > $GPIO/export
echo 43 > $GPIO/export
echo 44 > $GPIO/export

echo out > $LED3/direction
echo out > $LED4/direction
echo out > $LED5/direction

echo 1 > $LED3/value
sleep 1
echo 1 > $LED4/value
sleep 1
echo 1 > $LED5/value
sleep 1

echo 0 > $LED3/value
sleep 1
echo 0 > $LED4/value
sleep 1
echo 0 > $LED5/value

echo 42 > $GPIO/unexport
echo 43 > $GPIO/unexport
echo 44 > $GPIO/unexport
