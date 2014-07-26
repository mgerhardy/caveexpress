[![Build Status](https://travis-ci.org/mgerhardy/caveexpress.svg?branch=master)](https://travis-ci.org/mgerhardy/caveexpress)

### About

![Media](https://github.com/mgerhardy/caveexpress/raw/master/contrib/assets/media/caveexpress/950x500.png)

**[Homepage](http://www.caveproductions.org/)**

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
* make android-setup
* ./configure --target-os=android
* make

Compile for HTML5 (emscripten)
* make emscripten-setup
* ./configure --target-os=html5
* make

Compile for NaCl
* make nacl-setup
* ./configure --target-os=nacl
* make nacl-installer
* make nacl-start

### License
Code is released under the GPL3 and the game data is released
under CC BY-NC-SA 3.0
