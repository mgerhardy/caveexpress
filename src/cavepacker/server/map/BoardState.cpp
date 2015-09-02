#include "BoardState.h"
#include "common/Common.h"
#include <SDL_assert.h>

namespace cavepacker {

BoardState::BoardState() :
		_width(0), _height(0) {
}

void BoardState::clear() {
	_state.clear();
	_deadlock.clear();
}

void BoardState::setSize(int width, int height) {
	_width = width;
	_height = height;
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
	auto i = _state.find(index);
	if (i == _state.end()) {
		return false;
	}
	const char c = i->second;
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

void BoardState::getReachableIndices(int index, std::vector<int>& successors) const {
	int col, row;
	if (!getColRowFromIndex(index, col, row))
		return;
	const char dirs[] = { MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN };
	for (int i = 0; i < lengthof(dirs); ++i) {
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
		if (i->second == Sokoban::TARGET || i->second == Sokoban::PLAYERONTARGET)
			return false;
	}
	return true;
}

char BoardState::clearFieldForIndex(int index) {
	auto i = _state.find(index);
	if (i == _state.end())
		return '\0';
	const char field = i->second;
	_state.erase(i);
	return field;
}

char BoardState::clearField(int col, int row) {
	const int index = getIndex(col, row);
	return clearFieldForIndex(index);
}

bool BoardState::setField(int col, int row, char field) {
	const int index = getIndex(col, row);
	return setFieldForIndex(index, field);
}

bool BoardState::setFieldForIndex(int index, char field) {
	if (_state.find(index) != _state.end())
		return false;
	_state[index] = field;
	return true;
}

}
