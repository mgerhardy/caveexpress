#!/bin/sh

set -e
set -x

DIR=$(cd $(dirname $0); pwd)
cd $DIR/../../..
mkdir -p cp-build-osx
cd cp-build-osx
pwd

/Applications/CMake.app/Contents/bin/cmake -DCMAKE_BUILD_TYPE=Release -GXcode $DIR/../..

for i in caveexpress cavepacker; do
	echo "==========================================="
	echo "Building $i"
	echo "==========================================="
	echo ""
	echo ""
	xcodebuild build archive -project caveproductions.xcodeproj -configuration Release -archivePath $i.xcarchive -scheme $i | xcpretty
	xcodebuild -exportArchive -exportFormat app -archivePath $i.xcarchive -exportPath ${i}App | xcpretty
done
