#!/bin/sh

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

echo "Starting example: $dirTut"

dHere="$(pwd)"
dTool="$dHere/$(dirname $0)"
dTarget="$dTool/../../$dirTut/build-native"
dRelHereToTarget="$(realpath --relative-to=$dHere $dTarget)"
dRelTargetToTool="$(realpath --relative-to=$dTarget $dTool)"

echo "Target: $dRelHereToTarget"

#	--gen-suppressions=all \

cd "${dRelHereToTarget}" && \
ninja && \

valgrind \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-fds=yes \
	--suppressions=${dRelTargetToTool}/valgrind_suppressions.txt \
./app \
	$@

