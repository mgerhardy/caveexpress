#pragma once

#include "common/IMap.h"
#include "common/EntityType.h"
#include <string>

class EmitterDefinition {
public:
	// the coordinates of the emitter
	gridCoord x;
	gridCoord y;
	// the entity type to emit
	const EntityType* type;
	// the amount of entities that are emitted by this emitter
	int amount;
	// the time the emitter waits until it starts to emit the specified entity type
	int delay;
	// entities can specify different settings that your emitter entity implementation can use to
	// send parameters to the instantiation
	std::string settings;

	EmitterDefinition (gridCoord _x, gridCoord _y, const EntityType& _type, int _amount, int _delay, const std::string& _settings) :
			x(_x), y(_y), type(&_type), amount(_amount), delay(_delay), settings(_settings)
	{
	}

	inline std::string toString () const
	{
		std::stringstream ss;
		ss << "x: " << x << std::endl;
		ss << "y: " << y << std::endl;
		ss << "type: " << type->name << std::endl;
		ss << "amount: " << amount << std::endl;
		ss << "delay: " << delay << std::endl;
		ss << "settings: " << settings << std::endl;
		return ss.str();
	}
};
