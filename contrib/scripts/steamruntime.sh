#!/bin/bash

set -x
set -e

ARCH=${1:-amd64}
DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-steam-${ARCH}
cd cp-build-steam-${ARCH}
BINDIR=$(pwd)

echo "#!/bin/sh" > ${BINDIR}/script.sh
echo "cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${BINDIR}/steam $DIR/../.." >> ${BINDIR}/script.sh
echo "make -j 4" >> ${BINDIR}/script.sh
echo "make install" >> ${BINDIR}/script.sh
chmod +x script.sh
schroot --chroot steamrt_scout_${ARCH} -- ./script.sh
