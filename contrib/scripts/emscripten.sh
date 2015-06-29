#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../..
mkdir -p build-emscripten
cd build-emscripten
pwd
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/emscripten-toolchain.cmake
make $*

