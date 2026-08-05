#pragma once

#include "DeadlockTypes.h"
#include <vector>

namespace cavepacker {

class BoardState;
class SimpleDeadlockDetector;

/**
 * @brief Detects frozen package deadlocks.
 *
 * Keeps the original blocking rules (including treating the current package as a
 * wall while probing neighbors), but breaks mutual-recursion cycles with a visiting
 * set instead of re-entering the full board scan (which copied the board and
 * re-checked every package at every nesting level).
 */
class FrozenDeadlockDetector {
private:
	DeadlockSet _deadlocks;

	bool hasWallClose(const BoardState& s, int index) const;
	bool hasSimpleDeadlock(const SimpleDeadlockDetector& simple, int index) const;
	bool hasBlockedPackageClose(uint32_t millisStart, uint32_t millisTimeout,
			const SimpleDeadlockDetector& simple, BoardState& s,
			std::vector<uint8_t>& visiting, int neighborIndex, int origIndex);
	bool hasDeadlockVertically(uint32_t millisStart, uint32_t millisTimeout,
			const SimpleDeadlockDetector& simple, BoardState& s,
			std::vector<uint8_t>& visiting, int col, int row);
	bool hasDeadlockAt(uint32_t millisStart, uint32_t millisTimeout,
			const SimpleDeadlockDetector& simple, BoardState& s,
			std::vector<uint8_t>& visiting, int col, int row);
	bool isPackageFrozen(uint32_t millisStart, uint32_t millisTimeout,
			const SimpleDeadlockDetector& simple, BoardState& s,
			std::vector<uint8_t>& visiting, int index);

public:
	void clear();
	void init(const BoardState& s);
	bool hasDeadlock(uint32_t millisStart, uint32_t millisTimeout, const SimpleDeadlockDetector& simple, const BoardState& s);
	void fillDeadlocks(DeadlockSet& set) const;
};

inline void FrozenDeadlockDetector::clear() {
	_deadlocks.clear();
}

inline void FrozenDeadlockDetector::init(const BoardState&) {
}

inline void FrozenDeadlockDetector::fillDeadlocks(DeadlockSet& set) const {
	for (auto i = _deadlocks.begin(); i != _deadlocks.end(); ++i) {
		set.insert(*i);
	}
}

}
