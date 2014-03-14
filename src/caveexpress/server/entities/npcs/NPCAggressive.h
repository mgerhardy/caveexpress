#pragma once

#include "caveexpress/server/entities/npcs/NPC.h"

class NPCAggressive: public NPC {
public:
	NPCAggressive (const EntityType &type, Map& map);
	virtual ~NPCAggressive ();

	// NPC
	virtual const Animation& getFallingAnimation () const override;
	virtual const Animation& getIdleAnimation () const override;
	virtual bool shouldCollide (const IEntity* entity) const override;
};

inline const Animation& NPCAggressive::getFallingAnimation () const
{
	if (_lastDirectionRight)
		return Animations::ANIMATION_FALLING_RIGHT;
	return Animations::ANIMATION_FALLING_LEFT;
}

inline const Animation& NPCAggressive::getIdleAnimation () const
{
	if (_lastDirectionRight)
		return Animations::ANIMATION_IDLE_RIGHT;
	return Animations::ANIMATION_IDLE_LEFT;
}
