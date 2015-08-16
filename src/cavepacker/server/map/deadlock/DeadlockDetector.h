#pragma once

#include "cavepacker/server/map/BoardState.h"

namespace cavepacker {

/**
 * @brief Checks whether the current board contains a deadlock. There are several deadlock situations.
 *
 * You can find more information at http://sokobano.de/wiki/index.php?title=Deadlocks
 */
class DeadlockDetector {
public:
	static StateMap calculateDeadlockFields(const BoardState& state);
	static bool hasDeadlock(const BoardState& state);
};

}
