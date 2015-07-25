#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../../..
mkdir -p cp-build-linux
cd cp-build-linux
pwd
cmake $DIR/../..
make $*

