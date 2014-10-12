#!/bin/bash

set -e

EMSCRIPTEN_TARGET_ROOT=${1:-${HOME}}
PREFIX=${EMSCRIPTEN_TARGET_ROOT}/emsdk/sdl2

echo "Setup build environment for emscripten (install nodejs on your own)"
cd ${EMSCRIPTEN_TARGET_ROOT}
if [ ! -d emsdk ]; then
	wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
	tar -xzf emsdk-portable.tar.gz
	mv emsdk_portable emsdk
fi
cd emsdk
./emsdk update
./emsdk install latest
./emsdk activate latest
./emsdk_env.sh
source ./emsdk_set_env.sh
mkdir -p sdl-source
cd sdl-source
rm -rf SDL-emscripten
git clone https://github.com/Daft-Freak/SDL-emscripten.git
cd SDL-emscripten
	emconfigure ./configure --host=asmjs-unknown-emscripten --disable-assembly --disable-threads --enable-cpuinfo=false CFLAGS="-O2" --prefix=${PREFIX}
	emmake make -j 4
	make install
cd ..
rm -rf sdl2-image
git clone https://github.com/gsathya/sdl2-image.git
cd sdl2-image
	./autogen.sh
	emconfigure ./configure --disable-sdltest --with-sdl-prefix=${PREFIX} --prefix=${PREFIX}
	emmake make -j 4
	make install
cd ..
rm -rf SDL_mixer
hg clone http://hg.libsdl.org/SDL_mixer/
cd SDL_mixer
	./autogen.sh
	mkdir -p build && cd build
		EMCONFIGURE_JS=1 emconfigure ../configure --disable-sdltest --with-sdl-prefix=${PREFIX} --prefix=${PREFIX}
		emmake make -j 4
		make install
	cd ..
cd ..
cd ..
echo "Add the following line to your ~/.bashrc"
echo "source ${EMSCRIPTEN_TARGET_ROOT}/emsdk/emsdk_set_env.sh
