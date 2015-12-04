#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-linux
cd cp-build-linux
BINDIR=$(pwd)
#-DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE
cmake -G"Eclipse CDT4 - Unix Makefiles" -DMINIRACER=ON -DCMAKE_INSTALL_PREFIX=${BINDIR}/linux -DUSE_CCACHE=ON $DIR/../..
make $*
make install

