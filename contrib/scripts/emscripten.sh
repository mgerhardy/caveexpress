#!/bin/bash

set -e
set -x

EMSCRIPTEN_TARGET_ROOT=${1:-${HOME}}
PREFIX=${EMSCRIPTEN_TARGET_ROOT}/emsdk/sdl2
NODEVERSION="v0.10.32"

nodejs() {
	if [ ! -d node ]; then
		wget http://nodejs.org/dist/${NODEVERSION}/node-${NODEVERSION}-linux-x64.tar.gz
		tar -xzf node-${NODEVERSION}-linux-x64.tar.gz
		mv node-${NODEVERSION}-linux-x64 node
	fi
	export PATH=$PATH:${EMSCRIPTEN_TARGET_ROOT}/node/bin
}

emscripten() {
	if [ ! -d emsdk ]; then
		wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
		tar -xzf emsdk-portable.tar.gz
		mv emsdk_portable emsdk
	fi
	pushd emsdk
	./emsdk update
	./emsdk install latest
	./emsdk activate latest
	./emsdk_env.sh
	source ./emsdk_set_env.sh
}

emscripten_noupdate() {
	pushd emsdk
	./emsdk_env.sh
	source ./emsdk_set_env.sh
}

pushd ${EMSCRIPTEN_TARGET_ROOT}

echo "Setup build environment for emscripten"
emscripten
#emscripten_noupdate

echo "install nodejs"
nodejs

echo "Add the following line to your ~/.bashrc"
echo "source ${EMSCRIPTEN_TARGET_ROOT}/emsdk/emsdk_set_env.sh"
