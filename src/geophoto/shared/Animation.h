#pragma once

#include "common/NonCopyable.h"
#include "common/String.h"
#include <stdint.h>
#include <string>
#include <map>

class Animation: public NonCopyable {
public:
	typedef std::map<uint8_t, const Animation*> AnimationMap;
	typedef AnimationMap::const_iterator AnimationMapConstIter;
private:
	static AnimationMap _animations;
	static uint8_t _cnt;
	const bool _hasDirection;
	std::string _nameWithoutDirection;

	Animation (const std::string& _name, bool _loop = true) :
			id(_cnt++), name(_name), loop(_loop), _hasDirection(string::endsWith(_name, "-left") || string::endsWith(_name, "-right"))
	{
		_animations.insert(std::pair<uint8_t, Animation*>(id, this));

		if (!_hasDirection)
			_nameWithoutDirection = name;
		else if (string::endsWith(name, "-left"))
			_nameWithoutDirection = name.substr(0, name.length() - 5);
		else
			_nameWithoutDirection = name.substr(0, name.length() - 6);
	}

public:
	const uint8_t id;
	const std::string name;
	const bool loop;

	static Animation ANIMATION_NONE;
	// walking to the left
	static Animation ANIMATION_WALK_LEFT;
	// walking to the right
	static Animation ANIMATION_WALK_RIGHT;
	// standing and waiting for a taxi
	static Animation ANIMATION_IDLE;
	// falling into the water
	static Animation ANIMATION_FALLING;
	// swimming towards the player to get rescued
	static Animation ANIMATION_SWIMMING_LEFT;
	static Animation ANIMATION_SWIMMING_RIGHT;
	// struggle until getting rescued
	static Animation ANIMATION_SWIMMING_IDLE;
	// flying taxi animation
	static Animation ANIMATION_FLYING;
	// flying npc animations
	static Animation ANIMATION_FLYING_LEFT;
	static Animation ANIMATION_FLYING_RIGHT;
	// aggressive npc attacks
	static Animation ANIMATION_ATTACK_INIT_LEFT;
	static Animation ANIMATION_ATTACK_INIT_RIGHT;
	static Animation ANIMATION_ATTACK_LEFT;
	static Animation ANIMATION_ATTACK_RIGHT;
	static Animation ANIMATION_FALLING_LEFT;
	static Animation ANIMATION_FALLING_RIGHT;
	static Animation ANIMATION_IDLE_LEFT;
	static Animation ANIMATION_IDLE_RIGHT;
	static Animation ANIMATION_TURN_LEFT;
	static Animation ANIMATION_TURN_RIGHT;
	static Animation ANIMATION_KNOCKOUT_LEFT;
	static Animation ANIMATION_KNOCKOUT_RIGHT;
	static Animation ANIMATION_DAZED;
	static Animation ANIMATION_DAZED_LEFT;
	static Animation ANIMATION_DAZED_RIGHT;
	static Animation ANIMATION_WAKEUP_LEFT;
	static Animation ANIMATION_WAKEUP_RIGHT;
	static Animation ANIMATION_ACTIVE;
	static Animation ANIMATION_EXPLODE;
	static Animation ANIMATION_ROTATE;

	inline bool isNone () const
	{
		return *this == Animation::ANIMATION_NONE;
	}

	inline bool operator< (const Animation& other) const
	{
		return id < other.id;
	}

	inline bool operator== (const Animation& other) const
	{
		return id == other.id;
	}

	inline bool operator!= (const Animation& other) const
	{
		return !(*this == other);
	}

	static const Animation& get (uint8_t id)
	{
		AnimationMap::const_iterator i = _animations.find(id);
		if (i != _animations.end())
			return *i->second;

		return ANIMATION_NONE;
	}

	const std::string& getNameWithoutDirection () const
	{
		return _nameWithoutDirection;
	}

	inline bool hasDirection () const
	{
		return _hasDirection;
	}

	static inline AnimationMapConstIter begin ()
	{
		return _animations.begin();
	}

	static inline AnimationMapConstIter end ()
	{
		return _animations.end();
	}
};
