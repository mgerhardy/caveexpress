#pragma once

#include "DeadlockTypes.h"

namespace cavepacker {

class BoardState;
class SimpleDeadlockDetector;

class FrozenDeadlockDetector {
private:
	DeadlockSet _deadlocks;

	/**
	 * @brief Returns true if there is a package close to the given position (in the given direction) that
	 * is blocked
	 */
	bool hasAnyBlockedPackageClose(const SimpleDeadlockDetector& simple, BoardState& s, int col, int row, char dir);
	inline bool hasWallClose(const BoardState& s, int col, int row, char dir) const;
	inline bool hasSimpleDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s, int col, int row, char dir) const;
	bool hasDeadlock_(const SimpleDeadlockDetector& simple, BoardState& s, int col, int row);
public:
	void clear();
	void init(const BoardState& s);
	bool hasDeadlock(const SimpleDeadlockDetector& simple, const BoardState& s);

	/**
	 * @brief only add the first found deadlock
	 */
	void fillDeadlocks(DeadlockSet& set);
};

inline void FrozenDeadlockDetector::clear() {
	_deadlocks.clear();
}

inline void FrozenDeadlockDetector::init(const BoardState&) {
	// nothing to do here yet
}

inline void FrozenDeadlockDetector::fillDeadlocks(DeadlockSet& set) {
	for (auto i = _deadlocks.begin(); i != _deadlocks.end(); ++i) {
		set.insert(*i);
	}
}

}
