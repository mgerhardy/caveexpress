#include "CooldownMgr.h"
#include "common/Log.h"

namespace {
unsigned long durations[] = {
		0ul,
		15000ul,
		10000ul,
		100ul
};
CASSERT(lengthof(durations) == CooldownType::MAX);
}

CooldownMgr::CooldownMgr() {
}

CooldownTriggerState CooldownMgr::triggerCooldown(CooldownType type, unsigned long currentTime) {
	CooldownPtr c = cooldown(type);
	if (!c) {
		c = CooldownPtr(new Cooldown(type, defaultDuration(type)));
		_cooldowns[type] = c;
	} else if (c->running(currentTime)) {
		Log::debug(LOG_COOLDOWN, "Failed to trigger the cooldown of type %i: already running", type);
		return CooldownTriggerState::ALREADY_RUNNING;
	}
	c->start(currentTime);
	_queue.push(c);
	Log::debug(LOG_COOLDOWN, "Triggered the cooldown of type %i (expires in %lims, started at %li)",
			type, c->duration(), c->startMillis());
	return CooldownTriggerState::SUCCESS;
}

CooldownPtr CooldownMgr::cooldown(CooldownType type) const {
	auto i = _cooldowns.find(type);
	if (i == _cooldowns.end())
		return CooldownPtr();
	return i->second;
}

unsigned long CooldownMgr::defaultDuration(CooldownType type) const {
	return durations[type];
}

bool CooldownMgr::resetCooldown(CooldownType type) {
	const CooldownPtr& c = cooldown(type);
	if (!c) {
		return false;
	}
	c->reset();
	return true;
}

bool CooldownMgr::cancelCooldown(CooldownType type) {
	const CooldownPtr& c = cooldown(type);
	if (!c) {
		return false;
	}
	c->cancel();
	return true;
}

bool CooldownMgr::isCooldown(CooldownType type, unsigned long currentTime) {
	const CooldownPtr& c = cooldown(type);
	if (!c || !c->running(currentTime)) {
		Log::trace(LOG_COOLDOWN, "Cooldown of type %i is not running", type);
		return false;
	}
	Log::debug(LOG_COOLDOWN, "Cooldown of type %i is running and has a runtime of %lims", type, c->duration());
	return true;
}

void CooldownMgr::update(unsigned long currentTime) {
	for (;;) {
		if (_queue.empty()) {
			break;
		}
		CooldownPtr cooldown = _queue.top();
		if (cooldown->running(currentTime)) {
			break;
		}

		_queue.pop();
		Log::debug(LOG_COOLDOWN, "Cooldown of type %i has just expired at %li", cooldown->type(), currentTime);
		cooldown->expire();
	}
}
