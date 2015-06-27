#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../..
mkdir -p build-windows
cd build-windows
pwd
cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/${USER}/dev/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake
make -j 4

