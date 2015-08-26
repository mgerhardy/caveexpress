#include "BoardState.h"
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

void BoardState::initDeadlock() {
	_deadlock.init(*this);
}

bool BoardState::hasDeadlock() {
	return _deadlock.hasDeadlock(*this);
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
