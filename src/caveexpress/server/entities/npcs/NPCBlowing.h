#pragma once

#include "caveexpress/server/entities/npcs/NPC.h"

namespace caveexpress {

// forward decl
class Map;
class WindModificator;

class NPCBlowing: public NPC {
protected:
	WindModificator* _modificator;
	SpriteDefPtr _spriteDef;
public:
	NPCBlowing (Map& map, const b2Vec2& pos, bool right, float force, float modificatorSize);
	virtual ~NPCBlowing ();

	// NPC
	void update (uint32_t deltaTime) override;
	bool shouldCollide (const IEntity* entity) const override;
	const Animation& getIdleAnimation () const override;
	float getDensity () const override;
};

inline const Animation& NPCBlowing::getIdleAnimation () const
{
	if (_lastDirectionRight)
		return Animations::ANIMATION_IDLE_RIGHT;
	return Animations::ANIMATION_IDLE_LEFT;
}

}
