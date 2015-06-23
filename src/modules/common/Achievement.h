#pragma once

#include "common/System.h"
#include "common/Enum.h"
#include "common/Log.h"

class Achievement: public Enum<Achievement> {
private:
	bool _increment;
public:
	Achievement(const std::string& achievementId, bool increment = false) :
			Enum<Achievement>(achievementId), _increment(increment) {
	}

	/**
	 * Check whether the underlying system supports this particular achievement.
	 */
	bool supported() const {
		return System.hasAchievement(name);
	}

	bool unlock() const {
		if (!supported()) {
			Log::error(LOG_GENERAL, "achievement " + name + " is not supported");
			return false;
		}
		Log::error(LOG_GENERAL, "unlocking achievement " + name);
		System.achievementUnlocked(name, _increment);
		return true;
	}
};
