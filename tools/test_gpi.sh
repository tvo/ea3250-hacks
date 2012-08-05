#!/bin/bash
# Author: Tobi Vollebregt

if [ x"$1" = x"" ] || [ x"$2" = x"" ]; then
	echo "Usage: $0 <pin id> <pin name>"
	exit 0
fi

cd /sys/class/gpio
echo $1 > export
trap "echo $1 > unexport; exit 0" 2

[ -e $2/direction ] && echo in > $2/direction

while true; do
	value=`cat $2/value`
	echo -n "value : $value" $'\r'
	sleep 0.1
done
