#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-android
cd cp-build-android
pwd
cmake $DIR/../.. -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake
make $*
