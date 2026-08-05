#pragma once

#include "DeadlockTypes.h"
#include <vector>

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
	/** Indexed by board cell; faster than unordered_set::find in hot paths. */
	std::vector<uint8_t> _deadlockFlags;
	bool pull(char direction, BoardState& s, int index);
	bool moveBackwards(BoardState& s, int index);
public:
	void clear();
	int init(const BoardState& s);
	bool hasDeadlock(uint32_t millisStart, uint32_t millisTimeout, const BoardState& s) const;
	bool hasDeadlockAt(int index) const;
	void fillDeadlocks(DeadlockSet& set) const;
};

inline bool SimpleDeadlockDetector::hasDeadlockAt(int index) const {
	if (index < 0 || index >= (int)_deadlockFlags.size())
		return false;
	return _deadlockFlags[index] != 0;
}

inline void SimpleDeadlockDetector::fillDeadlocks(DeadlockSet& set) const {
	for (int index : _deadlocks) {
		set.insert(index);
	}
}

inline void SimpleDeadlockDetector::clear() {
	_visited.clear();
	_deadlocks.clear();
	_deadlockFlags.clear();
}

}
