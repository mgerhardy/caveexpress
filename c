#!/bin/bash

set -x

ulimit -c unlimited
rm -f core
set -e
make
set +e
./caveexpress $*
if [ -e core ]; then
	gdb ./caveexpress ./core
fi
