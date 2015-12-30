#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-windows
cd cp-build-windows
pwd
cmake $DIR/../.. -DCMAKE_TOOLCHAIN_FILE=${HOME}/dev/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake
make $*
