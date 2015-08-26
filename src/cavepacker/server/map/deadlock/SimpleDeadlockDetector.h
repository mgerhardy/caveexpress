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
	bool hasDeadlock(const BoardState& s) const;
	void fillDeadlocks(DeadlockSet& set);
};

}
