#!/bin/bash

dir=${0%/*}
if [ -d "$dir" ]; then
  pushd "$dir"
fi

pushd ../../../cp-build-osx

PROJECT=${1:-caveexpress}
ICONDIR=$PROJECT.app/Contents/Resources/$PROJECT.iconset
ORIGICON=../caveexpress/contrib/$PROJECT-icon-512x512.png

mkdir -p $ICONDIR

# Normal screen icons
for SIZE in 16 32 64 128 256 512; do
	sips -z $SIZE $SIZE $ORIGICON --out $ICONDIR/icon_${SIZE}x${SIZE}.png
done

# Retina display icons
for SIZE in 16 32 64 128 256 512; do
	sips -z $SIZE $SIZE $ORIGICON --out $ICONDIR/icon_$(expr $SIZE / 2)x$(expr $SIZE / 2)@2.png
done

# Make a multi-resolution Icon
iconutil -c icns -o $PROJECT.app/Contents/Resources/$PROJECT.icns $ICONDIR
rm -rf $ICONDIR #it is useless now
