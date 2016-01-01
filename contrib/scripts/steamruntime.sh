#!/bin/bash

set -x
set -e

DIR=$(dirname $(readlink -f $0))
cd $DIR/../..

APP=${1:-caveexpress}
ARCH=amd64

schroot --chroot steamrt_scout_${ARCH} -- ls && mkdir ../cp-build-steam && cd ../cp-build-steam && cmake ../caveexpress && make
