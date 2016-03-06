#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-ios
cd cp-build-ios
BINDIR=$(pwd)

/Applications/CMake.app/Contents/bin/cmake -DCMAKE_INSTALL_PREFIX=${BINDIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/ios-toolchain.cmake -GXcode $DIR/../..

xcodebuild build -target install -project caveproductions.xcodeproj -configuration Release | xcpretty
