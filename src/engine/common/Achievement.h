#pragma once

#include "engine/common/System.h"
#include "engine/common/Enum.h"
#include "engine/common/Logger.h"

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
		if (!supported()) {
			error(LOG_GENERAL, "achievement " + name + " is not supported");
			return false;
		}
		error(LOG_GENERAL, "unlocking achievement " + name);
		System.achievementUnlocked(name, _increment);
		return true;
	}
};
