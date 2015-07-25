#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-emscripten
cd cp-build-emscripten
pwd
cmake $DIR/../.. -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/emscripten-toolchain.cmake
make $*

