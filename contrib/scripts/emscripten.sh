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

sdl2() {
	if [ ! -d SDL-emscripten ]; then
		git clone https://github.com/Daft-Freak/SDL-emscripten.git
		pushd SDL-emscripten
	else
		pushd SDL-emscripten
		git pull --rebase
	fi
	emconfigure ./configure --host=asmjs-unknown-emscripten --disable-assembly --disable-threads --enable-cpuinfo=false CFLAGS="-O2" --prefix=${PREFIX}
	emmake make -j 4
	make install
	popd
}

sdl2_image() {
	if [ ! -d sdl2-image ]; then
		git clone https://github.com/gsathya/sdl2-image.git
		pushd sdl2-image
	else
		pushd sdl2-image
		git pull --rebase
	fi
	./autogen.sh
	emconfigure ./configure --disable-sdltest --with-sdl-prefix=${PREFIX} --prefix=${PREFIX}
	emmake make -j 4
	make install
	popd
}

sdl2_mixer() {
	if [ ! -d SDL_mixer ]; then
		hg clone http://hg.libsdl.org/SDL_mixer/
		pushd SDL_mixer
	else
		pushd SDL_mixer
		hg pull
		hg update
	fi
	./autogen.sh
	mkdir -p build
	pushd build
	EMCONFIGURE_JS=1 emconfigure ../configure --disable-sdltest --with-sdl-prefix=${PREFIX} --prefix=${PREFIX}
	emmake make -j 4
	make install
	popd
	popd
}


pushd ${EMSCRIPTEN_TARGET_ROOT}

echo "Setup build environment for emscripten"
#emscripten
emscripten_noupdate

echo "install nodejs"
nodejs

echo "build sdl"
mkdir -p sdl-source
pushd sdl-source
sdl2
sdl2_image
sdl2_mixer
popd

echo "Add the following line to your ~/.bashrc"
echo "source ${EMSCRIPTEN_TARGET_ROOT}/emsdk/emsdk_set_env.sh"
