#pragma once

namespace cavepacker {

namespace Sokoban {
const int WALL = '#';
const int PLAYER = '@';
const int PACKAGE = '$';
const int TARGET = '.';
const int GROUND = ' ';
const int PACKAGEONTARGET = '*';
const int PLAYERONTARGET = '+';
const int DEADLOCK = '\1';
}

static inline bool isWall(char c) {
	return Sokoban::WALL == c;
}

static inline bool isPlayer(char c) {
	return Sokoban::PLAYER == c;
}

static inline bool isPackage(char c) {
	return Sokoban::PACKAGE == c;
}

static inline bool isTarget(char c) {
	return Sokoban::TARGET == c;
}

static inline bool isGround(char c) {
	return Sokoban::GROUND == c;
}

static inline bool isPackageOnTarget(char c) {
	return Sokoban::PACKAGEONTARGET == c;
}

static inline bool isPlayerOnTarget(char c) {
	return Sokoban::PLAYERONTARGET == c;
}

}
