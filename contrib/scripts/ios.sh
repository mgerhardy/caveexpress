#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-ios
cd cp-build-ios
pwd

IOS_PROVISIONG_PROFILES_DIR="${HOME}/Library/MobileDevice/Provisioning Profiles/"

RESULTS=$(grep -l "Martin Gerhardy" "${IOS_PROVISIONG_PROFILES_DIR}"/*.mobileprovision)

/Applications/CMake.app/Contents/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/ios-toolchain.cmake -GXcode $DIR/../..

for i in caveexpress cavepacker; do
	echo "==========================================="
	echo "Building $i"
	echo "==========================================="
	echo ""
	echo ""
	xcodebuild build archive -sdk iphoneos -project caveproductions.xcodeproj -configuration Release -archivePath $i.xcarchive -scheme $i | xcpretty
	xcodebuild -exportArchive -exportFormat ipa -archivePath $i.xcarchive -exportPath $i.ipa -exportProvisioningProfile "${RESULTS}" | xcpretty
done
