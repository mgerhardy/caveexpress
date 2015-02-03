[![Build Status](https://travis-ci.org/mgerhardy/caveexpress.svg?branch=master)](https://travis-ci.org/mgerhardy/caveexpress)

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

#### Compilation

Requirements
* SDL2 >= 2.0.4
* SDL2_mixer
* SDL2_net
* SDL2_image
* sqlite3
* tinyxml2

On debian based systems do:
 sudo apt-get install libsdl2-mixer-dev libsdl2-net-dev libsdl2-image-dev libglm-dev libgtest-dev libbox2d-dev  libsqlite3-dev liblua5.2-dev libyajl-dev libtinyxml2-de

Additionally you could install the following packages:
 sudo apt-get install binutils-dev libncurses5-dev pngcrush imagemagick

libncurses is only needed if you run caveexpress with -server as argument. Then you would get a headless interface for setting up a server.
binutils-dev is only needed if you want to support stacktraces on crashes.
pngcrush is only needed if you plan to add your own images and want to execute the make target to crush all the png files.
imagemagick is also only needed if you plan to modify and add your own images and use existing makefile targets.

If you want to rebuild the texture atlas, then you also need TexturePacker

* ./configure
* make

Cross compile from linux for windows:
* Set up a cross compile tool chain like MXE
** git clone https://github.com/mxe/mxe.git
** cd mxe && make sdl2 sdl2_image sdl2_mixer
** echo "PATH=path/to/mxe/usr/bin/" ~/.mxe.settings
* ./configure --target-os=mingw64
* make

Cross compile for android
* make android-setup
* ./configure --target-os=android
* make

If you get an aapt - no such file or directory on 64bit debian based
distributions, do the following:
 sudo apt-get install lib32stdc++6
 sudo apt-get install lib32z1

Compile for HTML5 (emscripten)
* make emscripten-setup
* ./configure --target-os=html5
* make

Compile for NaCl
* make nacl-setup
* ./configure --target-os=nacl
* make nacl-installer
* make nacl-start

#### CavePacker

![Media](https://github.com/mgerhardy/caveexpress/raw/master/contrib/assets/media/cavepacker/screenshot-microban3.png)

CavePacker is a sokoban game. It's just a few days old and only the basic things are working. This was meant as an experiment on how
easy it would be to create a totally different game with the CaveExpress code base.

The maps are taken from XSokoban, KSokoban - the author of sasquatch and microban maps is David W. Skinner and the maps gri* are made by GRIGoRusha.

Btw.: Did I mention that this is a sokoban game that even supports network based multiplayer gaming. Check it out.

The multiplayer maps were all created by me and are released in the Public Domain.

### License
Code is released under the GPL3 and the game data is released
under CC BY-SA 4.0
