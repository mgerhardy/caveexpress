#!/bin/bash

BUILD_TYPE=Release
if [ "$ANDROID_RELEASE" == "OFF" ]; then
	BUILD_TYPE=Debug
fi
DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-android
cd cp-build-android
cmake $DIR/../.. -DTOOLS=OFF -DUNITTESTS=OFF \
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	-DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/android-toolchain.cmake
make $*

STATE=$(${ANDROID_SDK_ROOT}/platform-tools/adb get-state)
if [ "$STATE" == "device" ]; then
	cd ../cp-build-android
	make android-caveexpress-install android-cavepacker-install
fi
