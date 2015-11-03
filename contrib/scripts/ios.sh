#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
ARCHIVE_PATH="caveproductions.xcarchive"
EXPORT_PATH="export"
cd $DIR/../../..
mkdir -p cp-build-ios
cd cp-build-ios
pwd
/Applications/CMake.app/Contents/bin/cmake -DRELEASE=ON -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/ios-toolchain.cmake -GXcode $DIR/../..
xcodebuild build archive -sdk iphoneos -project caveproductions.xcodeproj -configuration Release -target ALL_BUILD $*
xcodebuild -exportArchive -exportFormat IPA -archivePath "${ARCHIVE_PATH}" -exportPath "${EXPORT_PATH}" -exportProvisioningProfile ""
