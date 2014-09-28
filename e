#!/bin/bash

set -e
set -x

ulimit -c unlimited
rm -f core
mkdir -p build
cd build
cmake ..
make
cd ..
./build/caveexpress -ui_push editor -set grabmouse false $*
if [ -e core ]; then
	gdb ./build/caveexpress ./core
fi
