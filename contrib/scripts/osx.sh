#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-osx
cd cp-build-osx
BINDIR=$(pwd)

/Applications/CMake.app/Contents/bin/cmake -DCMAKE_INSTALL_PREFIX=${BINDIR} -DCMAKE_BUILD_TYPE=Release -GXcode $DIR/../..

xcodebuild build -target install -project caveproductions.xcodeproj -configuration Release | xcpretty
