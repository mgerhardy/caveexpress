#pragma once

#include "CooldownType.h"
#include "CooldownTriggerState.h"

#include <memory>

class Cooldown {
private:
	CooldownType _type;
	unsigned long _durationMillis;
	unsigned long _startMillis;
	unsigned long _expireMillis;

public:
	Cooldown(CooldownType type, unsigned long durationMillis) :
			_type(type), _durationMillis(durationMillis), _startMillis(0ul), _expireMillis(0ul) {
	}

	inline void start(unsigned long currentTime) {
		_startMillis = currentTime;
		_expireMillis = _startMillis + _durationMillis;
	}

	inline void reset() {
		_startMillis = 0ul;
		_expireMillis = 0ul;
	}

	inline void expire() {
		reset();
	}

	inline void cancel() {
		reset();
	}

	unsigned long durationMillis() const {
		return _durationMillis;
	}

	inline bool started() const {
		return _expireMillis > 0ul;
	}

	inline bool running(unsigned long currentTime) const {
		return _expireMillis > 0ul && currentTime < _expireMillis;
	}

	inline unsigned long duration() const {
		return _expireMillis - _startMillis;
	}

	inline unsigned long startMillis() const {
		return _startMillis;
	}

	inline CooldownType type() const {
		return _type;
	}

	inline bool operator<(const Cooldown& rhs) const {
		return _expireMillis < rhs._expireMillis;
	}
};

typedef std::shared_ptr<Cooldown> CooldownPtr;

namespace std {
template<> struct hash<Cooldown> {
	inline size_t operator()(const Cooldown &c) const {
		return std::hash<size_t>()(static_cast<size_t>(c.type()));
	}
};
}
