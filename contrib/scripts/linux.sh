#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-linux
cd cp-build-linux
BINDIR=$(pwd)
#-DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE
cmake --version
cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_INSTALL_PREFIX=${BINDIR}/linux $DIR/../..
make $*
make install
