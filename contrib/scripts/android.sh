#!/bin/bash

BUILD_TYPE=Release
if [ "$ANDROID_RELEASE" == "OFF" ]; then
	BUILD_TYPE=Debug
fi
INSTALL_PACKAGES=${ANDROID_INSTALL_PACKAGES:-OFF}
DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
echo "build non hd version"
mkdir -p cp-build-android
cd cp-build-android
cmake $DIR/../.. -DTOOLS=OFF -DUNITTESTS=OFF -DANDROID_INSTALL_PACKAGES=${INSTALL_PACKAGES} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake
make $*
cd ..
echo "build hd version"
mkdir -p cp-build-android-hd
cd cp-build-android-hd
cmake $DIR/../.. -DTOOLS=OFF -DUNITTESTS=OFF -DANDROID_INSTALL_PACKAGES=${INSTALL_PACKAGES} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake -DMINIRACER=OFF -DCAVEPACKER=OFF -DHD_VERSION=ON
make $*
