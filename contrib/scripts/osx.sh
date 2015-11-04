#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-osx
cd cp-build-osx
pwd
/Applications/CMake.app/Contents/bin/cmake -DCMAKE_BUILD_TYPE=Release -GXcode $DIR/../..
xcodebuild build -project caveproductions.xcodeproj $* | xcpretty
