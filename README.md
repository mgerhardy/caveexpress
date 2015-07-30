[![Build Status](https://travis-ci.org/mgerhardy/caveexpress.svg?branch=master)](https://travis-ci.org/mgerhardy/caveexpress)
[![Build status](https://ci.appveyor.com/api/projects/status/5milbiquto4e6u2t?svg=true)](https://ci.appveyor.com/project/mgerhardy/caveexpress)

### General

This code repository contains the code and gamedata for two games

**[Homepage](http://www.caveproductions.org/)**

### CaveExpress

![Media](https://github.com/mgerhardy/caveexpress/raw/master/contrib/assets/media/caveexpress/950x500.png)

CaveExpress is a classic 2D platformer with physics-based gameplay
and dozens of levels. Master your pedal-powered flying machine to
pick up packages from your cave-dwelling clients and drop them off
at the collection point. But beware! Mighty mastodons, terrifying
pterodactyls and others would rather see you extinct.

Features:
* Multiplayer
* Built-in mapeditor
* Physics-based gameplay


#### CavePacker

![Media](https://github.com/mgerhardy/caveexpress/raw/master/contrib/assets/media/cavepacker/screenshot-microban3.png)

CavePacker is a sokoban game.
It is a puzzle game where you have to move the packages onto their targets without getting stuck. The lesser steps you made for solving a level, the better.
You may only push but not pull packages.

The maps are taken from XSokoban, KSokoban - the author of sasquatch and microban maps is David W. Skinner and the maps gri* are made by GRIGoRusha.

Btw.: Did I mention that this is a sokoban game that even supports network based multiplayer gaming. Check it out.

The multiplayer maps were all created by me and are released in the Public Domain.

### Compilation

#### Requirements
* cmake >= 2.8.7
* gcc or clang as compiler (msvc not yet supported/tested)
* SDL2 >= 2.0.3
* SDL2_mixer
* SDL2_net
* SDL2_image
* sqlite3
* glm
* lua >= 5.2
* Box2D
* yajl (only if you compile tools)

On debian based systems do:
 sudo apt-get install libbox2d-dev libyajl-dev libglm-dev libgtest-dev libsdl2-dev \
 libsdl2-net-dev libsdl2-image-dev libsdl2-mixer-dev libsqlite3-dev liblua5.2-dev zlib1g-dev

Additionally you could install the following packages:
 sudo apt-get install binutils-dev libncurses5-dev

libncurses is only needed if you run caveexpress with -server as argument. Then you would get a headless interface for setting up a server.
binutils-dev is only needed if you want to support stacktraces on crashes.

If you want to rebuild the texture atlas, then you also need TexturePacker. There is a custom exporter for the format we use in contrib/assets/png-packed/exporter.

#### Compile:
* mkdir build
* cd build
* cmake ..
* make
or simply
* ./contrib/scripts/linux.sh

#### Cross compile from linux for windows:
* Set up a cross compile tool chain like MXE
** git clone https://github.com/mxe/mxe.git
** cd mxe && make sdl2 sdl2_image sdl2_mixer box2d sdl2_net
* mkdir build
* cd build
* cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake
* make
or simply
* ./contrib/scripts/windows.sh

#### Cross compile for android
* install sdk and ndk
* mkdir build
* cd build
* cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/android-toolchain.cmake
* make
or simply:
* ./contrib/scripts/android.sh

If you get an aapt - no such file or directory on 64bit debian based
distributions, do the following:
 sudo apt-get install lib32stdc++6
 sudo apt-get install lib32z1

#### Compile for HTML5 (emscripten)
* install emsdk
* mkdir build-emscripten
* cd build-emscripten
* cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/emscripten-toolchain.cmake
* make
or simply:
* ./contrib/scripts/emscripten.sh

#### Compile for NaCl (outdated)
* make nacl-setup
* ./configure --target-os=nacl
* make nacl-installer
* make nacl-start

#### Compile for iOS (TODO)
* mkdir build-ios
* cd build-ios
* cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/ios-toolchain.cmake
* make

#### Compile for MacOSX
* mkdir build-macosx
* cd build-macosx
* cmake ..
* make

### License
Code is released under the GPL3 and the game data is released under CC BY-SA 4.0
