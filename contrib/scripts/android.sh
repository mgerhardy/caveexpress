#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-android
cd cp-build-android
cmake $DIR/../.. -DRELEASE=ON -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake
make $*
cd ..
mkdir -p cp-build-android-hd
cd cp-build-android-hd
cmake $DIR/../.. -DRELEASE=ON -DMINIRACER=OFF -DCAVEPACKER=OFF -DHD_VERSION=ON -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake
make $*
