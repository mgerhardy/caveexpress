#!/bin/bash

./configure --enable-ccache
make clean
make -j 4
if [ ! -f ./caveexpress ]; then
	exit 125
fi

./caveexpress

ANSWER=""

echo "-----------------------------------"
while [ "$ANSWER" != "yes" -a "$ANSWER" != "no" -a "$ANSWER" != "idk" -a "$ANSWER" != "exit" ]; do
	echo "Do you have the problem? Answer with [yes|no|idk|exit]"
	read ANSWER
done

case "$ANSWER" in
	yes) exit 1
	;;
	no) exit 0
	;;
	idk) exit 125
	;;
	*) exit 255
	;;
esac
