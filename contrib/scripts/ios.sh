#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-ios
cd cp-build-ios
pwd
/Applications/CMake.app/Contents/bin/cmake -DRELEASE=ON -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/ios-toolchain.cmake -GXcode $DIR/../..
${DIR}/xcode_schemes.rb
xcodebuild archive -sdk iphoneos -project caveproductions.xcodeproj -configuration Release -archivePath caveexpress.xcarchive -scheme caveexpress $*
xcodebuild archive -sdk iphoneos -project caveproductions.xcodeproj -configuration Release -archivePath caveexpress.xcarchive -scheme caveexpress $*
