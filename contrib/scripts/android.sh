#!/bin/bash

RELEASE=${ANDROID_RELEASE:-ON}
INSTALL_PACKAGES=${ANDROID_INSTALL_PACKAGES:-OFF}
DIR=$(dirname $(readlink -f $0))
echo $RELEASE
cd $DIR/../../..
mkdir -p cp-build-android
cd cp-build-android
cmake $DIR/../.. -DANDROID_INSTALL_PACKAGES=${INSTALL_PACKAGES} -DRELEASE=${RELEASE} -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake
make $*
cd ..
mkdir -p cp-build-android-hd
cd cp-build-android-hd
cmake $DIR/../.. -DRELEASE=${RELEASE} -DMINIRACER=OFF -DCAVEPACKER=OFF -DHD_VERSION=ON -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake
make $*
