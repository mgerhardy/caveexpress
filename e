#!/bin/bash

set -e
set -x

ulimit -c unlimited
rm -f core
make
./caveexpress -ui_push editor $*
if [ -e core ]; then
	gdb ./caveexpress ./core
fi
