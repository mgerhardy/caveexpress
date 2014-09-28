#!/bin/bash

set -x

APP=${1:-caveexpress}

ulimit -c unlimited
rm -f core
set -e
mkdir -p build
cd build
cmake ..
make
cd ..
set +e
./build/${APP} -set grabmouse false $*
if [ -e core ]; then
	gdb ./build/${APP} ./core
fi
