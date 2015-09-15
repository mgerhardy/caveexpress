#include "BoardState.h"
#include "common/Common.h"
#include <SDL_assert.h>

namespace cavepacker {

BoardState::BoardState() :
		_width(0), _height(0) {
}

BoardState::BoardState(const BoardState& state) :
		_state(state._state), _width(state._width), _height(state._height) {
}

void BoardState::clear() {
	_state.assign(_width * _height, '\0');
	_deadlock.clear();
}

void BoardState::setSize(int width, int height) {
	_width = width;
	_height = height;
	_state.assign(_width * _height, '\0');
}

std::string BoardState::toString() const {
	std::string mapStr = "     ";
	mapStr.reserve((_height + 2) * (_width + 5));
	for (int col = 0; col < _width; col += 5) {
		mapStr.append("....+");
	}
	mapStr.append("\n");
	for (int row = 0; row < _height; ++row) {
		for (int col = 0; col < _width; ++col) {
			if (col == 0) {
				char buf[6];
				char c;
				if (row % 5 == 4)
					c = '+';
				else
					c = '.';
				cp_snprintf(buf, sizeof(buf), "%03i %c", row + 1, c);
				mapStr.append(buf);
			}
			auto i = _state[getIndex(col, row)];
			if (i == '\0') {
				mapStr.append(" ");
				continue;
			}
			const char str[2] = { i, '\0' };
			mapStr.append(str);
		}
		mapStr.append("\n");
	}
	mapStr.append("     ");
	for (int col = 0; col < _width; col += 5) {
		mapStr.append("....+");
	}
	return mapStr;
}

void BoardState::initDeadlock() {
	_deadlock.init(*this);
}

bool BoardState::hasDeadlock() {
	return _deadlock.hasDeadlock(*this);
}

bool BoardState::isFree(int col, int row) const {
	const int index = getIndex(col, row);
	return isFree(index);
}

bool BoardState::isFree(int index) const {
	const char c = _state[index];
	if (c == '\0') {
		return false;
	}
	return c == Sokoban::GROUND || c == Sokoban::TARGET || c == Sokoban::VISITED;
}

bool BoardState::isTarget(int col, int row) const {
	const int index = getIndex(col, row);
	const char c = _state[index];
	if (c == '\0') {
		return false;
	}
	return c == Sokoban::PACKAGEONTARGET || c == Sokoban::PLAYERONTARGET || c == Sokoban::TARGET;
}

bool BoardState::isPackage(int col, int row) const {
	const int index = getIndex(col, row);
	const char c = _state[index];
	if (c == '\0') {
		return false;
	}
	return c == Sokoban::PACKAGE || c == Sokoban::PACKAGEONTARGET;
}

void BoardState::getReachableIndices(int index, std::vector<int>& successors) const {
	int col, row;
	if (!getColRowFromIndex(index, col, row))
		return;
	const char dirs[] = { MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN };
	const int dirLength = lengthof(dirs);
	for (int i = 0; i < dirLength; ++i) {
		int x, y;
		getXY(dirs[i], x, y);
		if (!isFree(col + x, row + y))
			continue;
		successors.push_back(getIndex(col + x, row + y));
	}
}

bool BoardState::isDone() const {
	for (auto i = _state.begin(); i != _state.end(); ++i) {
		// if there is an empty target left, we are not yet done
		if (*i == Sokoban::TARGET || *i == Sokoban::PLAYERONTARGET)
			return false;
	}
	return true;
}

char BoardState::clearFieldForIndex(int index) {
	const char c = _state[index];
	if (c == '\0') {
		return c;
	}
	_state[index] = '\0';
	return c;
}

}
