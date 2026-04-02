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
dRelTargetToTool="$(realpath --relative-to=$dTarget $dTool)"

echo "Target: $dRelHereToTarget"

supp="--suppressions=../valgrind.supp.txt"
dirVulkan="../LibDspc/vulkan"
if [ -d "$dirVulkan" ]; then
	echo "Suppressing Vulkan errors"
	suppVulkan="../../$dirVulkan/vulkan.supp.txt"
	echo "suppVulkan = $suppVulkan"
	supp="$supp --suppressions=$suppVulkan"
fi

cd "${dRelHereToTarget}" && \
ninja && \

valgrind \
	--leak-check=full \
	--show-leak-kinds=all \
	--gen-suppressions=all \
	--track-fds=yes \
	$supp \
./app \
	$@

