#!/bin/sh

p="$(pidof log-catching)"
if [ -z "$p" ]; then
	echo "log-catching not running"
	exit 1
fi

kill -USR1 "$p"

