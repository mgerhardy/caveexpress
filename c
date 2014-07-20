#!/bin/bash

set -x

APP=${1:-caveexpress}

ulimit -c unlimited
rm -f core
set -e
make ${APP}
set +e
./${APP} -set fullscreen false -set width 800 -set height 600 -set debug true -set grabmouse false $*
if [ -e core ]; then
	gdb ./${APP} ./core
fi
