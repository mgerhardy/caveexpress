#include "NPCBlowing.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/Direction.h"
#include "caveexpress/server/entities/modificators/WindModificator.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

NPCBlowing::NPCBlowing (Map& map, const b2Vec2& pos, bool right, float force, float modificatorSize) :
		NPC(EntityTypes::NPC_BLOWING, map)
{
	_lastDirectionRight = right;

	createBody(pos);
	const Direction dir = right ? DIRECTION_RIGHT : DIRECTION_LEFT;
	_modificator = new WindModificator(_map, dir, force, modificatorSize, 1.2f);
	// now that we have the npc body we can query the size
	// and configure the modificator
	_modificator->createBody(pos, getSize().x / 4.0f);
	_spriteDef = SpriteDefinition::get().getFromEntityType(_type, getIdleAnimation());
}

NPCBlowing::~NPCBlowing ()
{
	delete _modificator;
}

bool NPCBlowing::shouldCollide (const IEntity* entity) const
{
	return entity->isSolid();
}

void NPCBlowing::update (uint32_t deltaTime)
{
	NPC::update(deltaTime);
	_modificator->update(deltaTime);
	if (_state == NPCState::NPC_DAZED) {
		_modificator->setModificatorState(false);
	} else {
		const bool active = SpriteDefinition::get().isFrameActive(_time, _spriteDef);
		_modificator->setModificatorState(active);
	}
	_modificator->setRelativePositionTo(getPos());
}

float NPCBlowing::getDensity () const
{
	return DENSITY_NPC_BLOWING;
}

}
