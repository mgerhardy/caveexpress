#!/bin/bash

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
cmake -DCMAKE_TOOLCHAIN_FILE=$DIR/../../cmake/toolchains/steamlink-toolchain.cmake -DCMAKE_INSTALL_PREFIX=${BINDIR}/steamlink $DIR/../..
make $*
make install

