#pragma once

#include "engine/common/System.h"
#include "engine/common/Enum.h"

class Achievement: public Enum<Achievement> {
private:
	bool _increment;
public:
	Achievement(const std::string& id, bool increment = false) :
			Enum<Achievement>(id), _increment(increment) {
	}

	/**
	 * Check whether the underlying system supports this particular achievement.
	 */
	bool supported() const {
		return System.hasAchievement(name);
	}

	bool unlock() {
		if (!supported())
			return false;
		System.achievementUnlocked(name, _increment);
		return true;
	}
};
