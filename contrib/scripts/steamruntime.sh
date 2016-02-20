#!/bin/bash

set -x
set -e

DIR=$(dirname $(readlink -f $0))

TARGET=${1:-steam}
USE_BUILTIN=${2:-OFF}
BUILD_TYPE=${3:-RelMinSize}
CMAKE_OPTIONS="-DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DUSE_BUILTIN=${USE_BUILTIN}"

for ARCH in amd64 i386; do
	cd $DIR/../../..
	mkdir -p cp-build-${TARGET}-${ARCH}
	cd cp-build-${TARGET}-${ARCH}
	BINDIR=$(pwd)

	echo "Building ${TARGET}-${ARCH}"
	echo "Use builtin: ${USE_BUILTIN}"
	echo "Build type: ${BUILD_TYPE}"
	echo "Bin dir: ${BIN_DIR}/${TARGET}"

	echo "#!/bin/sh" > ${BINDIR}/script.sh
	echo "cmake ${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=${BINDIR}/${TARGET} $DIR/../.." >> ${BINDIR}/script.sh
	echo "make -j 4" >> ${BINDIR}/script.sh
	echo "make install/strip" >> ${BINDIR}/script.sh
	chmod +x script.sh
	schroot --chroot steamrt_scout_${ARCH} -- ./script.sh
done
