#!/bin/bash

set -x

ulimit -c unlimited
rm -f core
set -e
make tests
set +e
./tests $*
if [ -e core ]; then
	gdb ./tests ./core
fi
