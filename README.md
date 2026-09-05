States for building on Linux and Mac

[![Actions Status](https://github.com/mgerhardy/caveexpress/actions/workflows/main.yml/badge.svg)](https://github.com/mgerhardy/caveexpress/actions)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**[Homepage](http://www.caveproductions.org/)**

**[Discord](https://discord.com/invite/XqNBRsBY)**


# About

This repository is the home of two games: CaveExpress and CavePacker

## CaveExpress

![CaveExpress](https://raw.githubusercontent.com/mgerhardy/caveexpress/master/contrib/assets/media/caveexpress/950x500.png)

CaveExpress is a classic 2D platformer with physics-based gameplay and dozens of levels. Master your pedal-powered flying machine to pick up packages from your cave-dwelling clients and drop them off at the collection point. But beware! Mighty mastodons, terrifying pterodactyls and others would rather see you extinct.

Features:

* Multiplayer
* Built-in map editor
* Physics-based gameplay

The map editor is available from the main menu (**Editor**) or via `caveexpress -ui_push editor`. See [docs/caveexpress/EDITOR.md](docs/caveexpress/EDITOR.md) for how to create maps, tools, and key bindings. Map Lua scripts and cutscenes: [docs/caveexpress/MAPS.md](docs/caveexpress/MAPS.md). Sprites, atlases, and drawing: [docs/caveexpress/SPRITES.md](docs/caveexpress/SPRITES.md).

Online version: [caveexpress](https://mgerhardy.github.io/caveexpress/caveexpress.html)

## CavePacker

![CavePacker](https://raw.githubusercontent.com/mgerhardy/caveexpress/master/contrib/assets/media/cavepacker/screenshot-microban3.png)

CavePacker is a sokoban game. It is a puzzle game where you have to move the packages onto their targets without getting stuck. The lesser steps you made for solving a level, the better. You may only push but not pull packages.

The maps are taken from XSokoban, KSokoban - the author of sasquatch and microban maps is David W. Skinner and the maps gri* are made by GRIGoRusha.

Btw.: Did I mention that this is a sokoban game that even supports network based multiplayer gaming. Check it out.

The multiplayer maps were all created by me and are released in the Public Domain or CC0.

Features:

* Multiplayer
* Built-in mapeditor

CavePacker includes a map editor. You can start it by executing it via `cavepacker -ui_push editor`

Online version: [cavepacker](https://mgerhardy.github.io/caveexpress/cavepacker.html)


# Installation

Use the nightly builds from [github actions](https://github.com/mgerhardy/caveexpress/actions) or install a stable release:

### Debian

`apt-get install caveexpress cavepacker`

### Other

Download from [github releases](https://github.com/mgerhardy/caveexpress/releases/tag/2.4)


# Development

## Dependencies

 * cmake >= 2.8.7
 * gcc, clang or msvc as compiler (C++11 support is needed)
 * SDL2 >= 2.0.3
 * SDL2_mixer >= 2.0.0
 * SDL2_net >= 2.0.0
 * SDL2_image >= 2.0.0
 * sqlite3
 * glm
 * lua >= 5.2
 * Box2D >= 2.4.1
 * yajl (only if you compile tools)

## Compilation

How to build projects from sources described on wiki page [Compilation](https://github.com/mgerhardy/caveexpress/wiki/Compilation).

### Android

CI builds arm64-v8a debug APKs with the NDK CMake toolchain and the Gradle project in `android-project/` (same layout as [libsdl-org/SDL](https://github.com/libsdl-org/SDL/tree/main/.github)).

```sh
# ANDROID_NDK_HOME must point at an NDK (r28c or compatible)
# ANDROID_SDK_ROOT is required to package APKs
./contrib/scripts/android.sh
```

`ANDROID_SKIP_APK=1` builds only the native `.so` files. `ANDROID_ABI` defaults to `arm64-v8a`. Helper targets after CMake configure: `android-caveexpress-apk`, `android-cavepacker-apk`.


# Tools

Tools used for development, included in [sources](https://github.com/mgerhardy/caveexpress/tree/master/src/caveexpress/tools).

## TextureAtlas

The physical shapes with [box2deditor](https://github.com/mgerhardy/box2d-editor)

To convert the `tps` files into a texture atlas you can use the bundled tool `textureatlas`. The `tps` files can get created with Texturepacker 3.9.4.

The tool can read `tps` files (to some extent) and generate the needed texture atlas and lua sprite definitions that are needed for cavepacker and caveexpress. Call the tool with `-h` as parameter to get an overview of the command line options.

To generate e.g. all needed images and lua scripts for caveexpress, you can do this:

```sh
./textureatlas contrib/assets/png/caveexpress*.tps
```

## jsonconvert

Converts the box2deditor json output into CaveExpress readable format.

## soundmapper

Generate the entitysound.lua file for CaveExpress.

## tiledgenerator

Generate tiled mapeditor tileset definitions.

## CVARs

There is an in-game console (open it with `SHIFT+TAB` where you can execute commands and show or change configuration variables)
