#include "BoardState.h"
#include "cavepacker/server/map/deadlock/DeadlockDetector.h"

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
	for (auto i = _state.begin(); i != _state.end(); ++i) {
		// if there is an empty target left, we are not yet done
		if (i->second == Sokoban::TARGET || i->second == Sokoban::PLAYERONTARGET)
			return false;
	}
	return true;
}

bool BoardState::setField(int col, int row, char field) {
	const int index = getIndex(col, row);
	if (_state.find(index) != _state.end())
		return false;
	_state[index] = field;
	return true;
}

bool BoardState::canMoveLeft(int col, int row) const {
	// TODO:
	return true;
}

bool BoardState::canMoveRight(int col, int row) const {
	// TODO:
	return true;
}

bool BoardState::canMoveUp(int col, int row) const {
	// TODO:
	return true;
}

bool BoardState::canMoveDown(int col, int row) const {
	// TODO:
	return true;
}

}
