#!/bin/bash

dirTut="t01_tcp-echo-server"

if [ ! -z "$1" ]; then
	inUser="$1"

	echo "User input: $inUser"

	if [ -d "$inUser" ]; then
		dirTut="$inUser"
	else
		dirTut="$(ls -1 | grep "$inUser" | sort | head -n 1)"
	fi
fi

echo "Building example: $dirTut"

if [ ! -d "$dirTut" ]; then
	echo "Could not build tutorial. Wrong directory?"
	exit 1
fi

cd "$dirTut"

if [ ! -d "build-native" ]; then
	meson setup build-native
fi
ninja -C build-native

