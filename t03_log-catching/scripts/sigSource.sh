#!/bin/sh

#ps -ef | grep -e "test" -e "sleep"

p1="$(ps -ef | grep "test.sh" | grep bash | tr -s ' ' | cut -d ' ' -f 2)"
if [ -z "$p1" ]; then
	echo "test script not running"
	exit 1
fi

p2="$(ps -ef | grep "$p1" | grep sleep | tr -s ' ' | cut -d ' ' -f 2)"
if [ -z "$p2" ]; then
	echo "sleep not running"
fi

#echo "$p1"
#echo "$p2"

#exit 1

kill -SEGV "$p1"
kill "$p2"

