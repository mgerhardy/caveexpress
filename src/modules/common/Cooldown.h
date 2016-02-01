#pragma once

#include "common/Enum.h"

class Cooldown: public Enum<Cooldown> {
private:
	uint32_t _runtime;
public:
	Cooldown(const std::string& _name) :
			Enum<Cooldown>(_name), _runtime(0u) {
	}

	void setRuntime(uint32_t runtime) {
		_runtime = runtime;
	}

	inline uint32_t getRuntime() const {
		return _runtime;
	}
};
