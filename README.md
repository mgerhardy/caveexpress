[![Build Status](https://travis-ci.org/mgerhardy/caveexpress.svg?branch=master)](https://travis-ci.org/mgerhardy/caveexpress)

### About
CaveExpress is a classic 2D platformer with physics-based gameplay
and dozens of levels. Master your pedal-powered flying machine to
pick up packages from your cave-dwelling clients and drop them off
at the collection point. But beware! Mighty mastodons, terrifying
pterodactyls and others would rather see you extinct.

Features:
* Multiplayer
* Built-in mapeditor
* Physics-based gameplay

### Compilation
* ./configure
* make

Cross compile from linux for windows:
* Set up a cross compile tool chain like MXE
* ./configure --target-os=mingw64
* make

Cross compile for android
* download latest ndk and sdk
* ./configure --target-os=android
* make

Compile for HTML5 (emscripten)
* download and set up emscripten
* ./configure --target-os=html5
* make

Compile for NaCl
* download the native client sdk from https://developer.chrome.com/native-client/sdk/download
* ./configure --target-os=nacl
* make

### License
Code is released under the GPL3 and the game data is released
under CC BY-NC-SA 3.0
