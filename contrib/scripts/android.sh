#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../..
mkdir -p build-android
cd build-android
pwd
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/android.cmake ..
make VERBOSE=1 -j 4

