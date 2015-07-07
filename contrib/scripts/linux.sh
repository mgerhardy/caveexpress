#!/bin/bash

DIR=$(dirname $(readlink -f $0))
cd $DIR/../..
mkdir -p build
cd build
pwd
cmake ..
make $*

