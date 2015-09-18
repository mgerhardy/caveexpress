#pragma once

#include "DeadlockTypes.h"

namespace cavepacker {

/**
 * @brief This class of deadlocks is static per puzzle. They are detected by trying to "pull" from
 * each target destination in all four directions until this doesn't work anymore. Afterwards the fields
 * that were not touched during these pulls are deadlock fields.
 */
class SimpleDeadlockDetector {
private:
	DeadlockSet _deadlocks;
	DeadlockSet _visited;
	bool pull(char direction, BoardState& s, int index);
	bool moveBackwards(BoardState& s, int index);
public:
	void clear();
	void init(const BoardState& s);
	bool hasDeadlock(uint32_t millisStart, uint32_t millisTimeout, const BoardState& s) const;
	bool hasDeadlockAt(int index) const;
	void fillDeadlocks(DeadlockSet& set) const;
};

inline bool SimpleDeadlockDetector::hasDeadlockAt(int index) const {
	return _deadlocks.find(index) != _deadlocks.end();
}

inline void SimpleDeadlockDetector::fillDeadlocks(DeadlockSet& set) const {
	for (auto i = _deadlocks.begin(); i != _deadlocks.end(); ++i) {
		set.insert(*i);
	}
}

inline void SimpleDeadlockDetector::clear() {
	_visited.clear();
	_deadlocks.clear();
}

}
