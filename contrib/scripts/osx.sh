#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-osx
cd cp-build-osx
pwd
/Applications/CMake.app/Contents/bin/cmake -DRELEASE=ON -GXcode $DIR/../..
xcodebuild archive -project caveproductions.xcodeproj -configuration Release -archivePath caveexpress.xcarchive -scheme caveexpress $*
xcodebuild archive -project caveproductions.xcodeproj -configuration Release -archivePath caveexpress.xcarchive -scheme caveexpress $*
