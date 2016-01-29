#pragma once

#include "Cooldown.h"
#include "common/NonCopyable.h"

#include <memory>
#include <unordered_map>
#include <queue>
#include <functional>
#include <vector>

/**
 * @brief Cooldown manager that handles cooldowns for one entity
 */
class CooldownMgr: public NonCopyable {
private:
	struct CooldownComparatorLess: public std::binary_function<CooldownPtr, CooldownPtr, bool> {
		inline bool operator()(const CooldownPtr x, const CooldownPtr y) const {
			return std::less<Cooldown>()(*x.get(), *y.get());
		}
	};

	typedef std::priority_queue<CooldownPtr, std::vector<CooldownPtr>, CooldownComparatorLess> CooldownQueue;
	CooldownQueue _queue;
	typedef std::unordered_map<CooldownType, CooldownPtr> Cooldowns;
	Cooldowns _cooldowns;
	CooldownPtr cooldown(CooldownType type) const;

public:
	CooldownMgr();

	/**
	 * @brief Tries to trigger the specified cooldown for the given entity
	 */
	CooldownTriggerState triggerCooldown(CooldownType type, unsigned long currentTime);

	/**
	 * @brief Reset a cooldown and restart it
	 */
	bool resetCooldown(CooldownType type);

	unsigned long defaultDuration(CooldownType type) const;

	/**
	 * @brief Cancel an already running cooldown
	 */
	bool cancelCooldown(CooldownType type);

	/**
	 * @brief Checks whether a user has the given cooldown running
	 */
	bool isCooldown(CooldownType type, unsigned long currentTime);

	/**
	 * @brief Update cooldown states
	 */
	void update(unsigned long currentTime);
};

typedef std::shared_ptr<CooldownMgr> CooldownMgrPtr;
