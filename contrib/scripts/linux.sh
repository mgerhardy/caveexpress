#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-linux
cd cp-build-linux
pwd
#-DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE
cmake -G"Eclipse CDT4 - Unix Makefiles" -DUSE_CCACHE=ON $DIR/../..
make $*

