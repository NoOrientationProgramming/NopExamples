#!/bin/sh

dirTut="t01_tcp-echo-server"

if [ -z "$1" ]; then
	echo "Usage: $0 t??"
	exit 1
fi

inUser="$1"
shift

echo "User input: $inUser"

if [ -d "$inUser" ]; then
	dirTut="$inUser"
else
	dirTut="$(ls -1 | grep "$inUser" | sort | head -n 1)"
fi

echo "Starting example: $dirTut"

dHere="$(pwd)"
dTool="$dHere/$(dirname $0)"
dTarget="$dTool/../../$dirTut/build-native"
dRelHereToTarget="$(realpath --relative-to=$dHere $dTarget)"

cd "${dRelHereToTarget}" && \
objdump -d -S ./app > listing.txt

