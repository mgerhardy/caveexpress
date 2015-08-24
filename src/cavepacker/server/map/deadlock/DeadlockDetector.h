#pragma once

#include "cavepacker/server/map/BoardState.h"
#include <unordered_map>

namespace cavepacker {

struct DeadlockState {
	BoardState state;
	// contains indices of packages that are already checked
	std::unordered_map<int, bool> checkedState;
};

/**
 * @brief Checks whether the current board contains a deadlock. There are several deadlock situations.
 *
 * You can find more information at http://sokobano.de/wiki/index.php?title=Deadlocks
 */
class DeadlockDetector {
private:
	static bool canMovePackageLeft(DeadlockState& state, int col, int row);
	static bool canMovePackageRight(DeadlockState& state, int col, int row);
	static bool canMovePackageUp(DeadlockState& state, int col, int row);
	static bool canMovePackageDown(DeadlockState& state, int col, int row);

	static bool canMovePackageDirection(DeadlockState& state, char dir, int col, int row);

public:
	/**
	 * @brief Checks whether the package on the given coordinates can be moved. This is doing a recursive check of packages
	 * @note It's assumed that the given col and row is occupied by a package.
	 */
	static bool canMovePackage(DeadlockState& state, int col, int row);
	static StateMap calculateDeadlockFields(DeadlockState& state);
	static bool hasDeadlock(const BoardState& state);
};

}
