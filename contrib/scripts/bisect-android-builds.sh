#!/bin/sh

./configure --target-os=android --enable-release --enable-hd --enable-ccache
make clean-android clean
make -j 4
if [ $? -eq 2 ]; then
	exit 1
fi
