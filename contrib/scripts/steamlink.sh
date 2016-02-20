#!/bin/bash

if [ -f ~/steamlink-sdk/setenv.sh ]; then
	. ~/steamlink-sdk/setenv.sh
fi

# Sanity checks
if [ -z "$MARVELL_SDK_PATH" ]; then
	echo "MARVELL_SDK_PATH is not set - import setenv_external.sh of the steamlink sdk"
	exit 1
fi

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-steamlink
cd cp-build-steamlink

BINDIR=$(pwd)
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/steamlink-toolchain.cmake -DCMAKE_INSTALL_PREFIX=${BINDIR}/steamlink $DIR/../..
make $*
make install/strip

if [ -n "$STEAMLINK_IP" ]; then
	echo "Copying the game over to the device"
	echo "You might need to copy the libSDL* and libpng* stuff manually over to /home/steam/caveexpress"
	echo "They are not on the device right now"
	scp -r steamlink/* root@$STEAMLINK_IP:/home/steam/apps
	cd $DIR/../..
	$MARVELL_SDK_PATH/scripts/create_gdbinit.sh caveexpress $STEAMLINK_IP:8080
	armv7a-cros-linux-gnueabi-gdb
fi

