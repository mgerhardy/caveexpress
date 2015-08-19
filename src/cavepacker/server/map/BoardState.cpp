#include "BoardState.h"
#include "cavepacker/server/map/deadlock/DeadlockDetector.h"
#include <SDL_assert.h>

namespace cavepacker {

BoardState::BoardState() :
		_width(0), _height(0) {
}

void BoardState::clear() {
	_state.clear();
}

void BoardState::setSize(int width, int height) {
	_width = width;
	_height = height;
}

std::string BoardState::toString() const {
	std::string mapStr;
	mapStr.reserve(_height * _width);
	for (int row = 0; row < _height; ++row) {
		for (int col = 0; col < _width; ++col) {
			auto i = _state.find(getIndex(col, row));
			if (i == _state.end()) {
				mapStr.append(" ");
				continue;
			}
			const char str[2] = { i->second, '\0' };
			mapStr.append(str);
		}
		mapStr.append("\n");
	}
	return mapStr;
}

bool BoardState::hasDeadlock() const {
	return DeadlockDetector::hasDeadlock(*this);
}

bool BoardState::isFree(int col, int row) const {
	const int index = getIndex(col, row);
	auto i = _state.find(index);
	if (i == _state.end()) {
		Log::debug(LOG_MAP, "col: %i, row: %i is not part of the map", col, row);
		return false;
	}
	const char c = i->second;
	Log::debug(LOG_MAP, "col: %i, row: %i is of type '%c'", col, row, c);
	return c == Sokoban::GROUND || c == Sokoban::TARGET;
}

bool BoardState::isTarget(int col, int row) const {
	const int index = getIndex(col, row);
	auto i = _state.find(index);
	if (i == _state.end()) {
		return false;
	}
	const char c = i->second;
	return c == Sokoban::PACKAGEONTARGET || c == Sokoban::PLAYERONTARGET || c == Sokoban::TARGET;
}

bool BoardState::isPackage(int col, int row) const {
	const int index = getIndex(col, row);
	auto i = _state.find(index);
	if (i == _state.end()) {
		return false;
	}
	const char c = i->second;
	return c == Sokoban::PACKAGE || c == Sokoban::PACKAGEONTARGET;
}

bool BoardState::isDone() const {
	const std::string& str = toString();
	const char* strc = str.c_str();
	Log::error(LOG_MAP, "%s", strc);
	for (auto i = _state.begin(); i != _state.end(); ++i) {
		// if there is an empty target left, we are not yet done
		if (i->second == Sokoban::TARGET || i->second == Sokoban::PLAYERONTARGET)
			return false;
	}
	return true;
}

char BoardState::clearField(int col, int row) {
	const int index = getIndex(col, row);
	auto i = _state.find(index);
	if (i == _state.end())
		return '\0';
	const char field = i->second;
	_state.erase(i);
	return field;
}

bool BoardState::setField(int col, int row, char field) {
	const int index = getIndex(col, row);
	if (_state.find(index) != _state.end())
		return false;
	_state[index] = field;
	return true;
}

bool BoardState::canMove(int col, int row) const {
	// we only need to check two directions here
	const bool left = canMoveLeft(col, row);
	const bool up = canMoveUp(col, row);
	const bool moveable = left || up;
	if (moveable)
		Log::info(LOG_MAP, "package on %i:%i can be moved", col, row);
	else
		Log::info(LOG_MAP, "package on %i:%i can't be moved", col, row);
	return moveable;
}

bool BoardState::canMoveDirection(char dir, int col, int row) const {
	int lx, ly;
	getXY(dir, lx, ly);
	int ox, oy;
	getOppositeXY(dir, ox, oy);
	if (!isFree(col + lx, row + ly) || !isFree(col + ox, row + oy)) {
		return false;
	}

	if (isPackage(col + lx, row + ly)) {
		return canMove(col + lx, row + ly);
	}
	if (isPackage(col + ox, row + oy)) {
		return canMove(col + ox, row + oy);
	}

	return true;
}

bool BoardState::canMoveLeft(int col, int row) const {
	return canMoveDirection(MOVE_LEFT, col, row);
}

bool BoardState::canMoveRight(int col, int row) const {
	return canMoveDirection(MOVE_RIGHT, col, row);
}

bool BoardState::canMoveUp(int col, int row) const {
	return canMoveDirection(MOVE_UP, col, row);
}

bool BoardState::canMoveDown(int col, int row) const {
	return canMoveDirection(MOVE_DOWN, col, row);
}

}
