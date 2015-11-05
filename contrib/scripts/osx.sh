#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-osx
cd cp-build-osx
pwd

# TODO: the path is incorrect
IOS_PROVISIONG_PROFILES_DIR="${HOME}/Library/MobileDevice/Provisioning Profiles/"

# TODO: extension is incorrect
RESULTS=$(grep -l "Martin Gerhardy" ${PROVISIONG_PROFILES_DIR}/*.mobileprovision)

/Applications/CMake.app/Contents/bin/cmake -DCMAKE_BUILD_TYPE=Release -GXcode $DIR/../..

for i in caveexpress cavepacker; do
	echo "==========================================="
	echo "Building $i"
	echo "==========================================="
	echo ""
	echo ""
	xcodebuild build archive -project caveproductions.xcodeproj -configuration Release -archivePath $i.xcarchive -scheme $i | xcpretty
	xcodebuild -exportArchive -exportFormat ipa -archivePath $i.xcarchive -exportPath $i.ipa -exportProvisioningProfile "${RESULTS}" | xcpretty
done
