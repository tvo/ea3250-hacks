#!/bin/bash
# Author: Tobi Vollebregt

set -e

if [ x"$1" = x"" ] || [ x"$2" = x"" ]; then
	echo "Usage: $0 <pin id> <pin name>"
	exit 0
fi

cd /sys/class/gpio
echo $1 > export
trap "echo $1 > unexport; exit 0" 2

[ -e $2/direction ] && echo out > $2/direction

value=0
while true; do
	value=$((1-value))
	echo -n "value : $value" $'\r'
	echo $value > $2/value
	sleep 3
done
