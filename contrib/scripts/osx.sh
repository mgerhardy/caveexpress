#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-osx
cd cp-build-osx
pwd
/Applications/CMake.app/Contents/bin/cmake -GXcode $DIR/../..
xcodebuild -project Project.xcodeproj -target ALL_BUILD $*
