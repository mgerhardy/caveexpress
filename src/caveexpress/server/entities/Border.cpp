#include "Border.h"
#include "caveexpress/server/entities/npcs/NPCAggressive.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

Border::Border (BorderType::Type borderType, Map& map, bool crashOnTouch) :
		IEntity(EntityTypes::BORDER, map), _borderType(borderType), _crashOnTouch(crashOnTouch)
{
}

Border::~Border ()
{
}

bool Border::shouldCollide (const IEntity *entity) const
{
	if (_borderType == BorderType::PLAYER_BOTTOM)
		return entity->isPlayer();
	return !entity->isNpcFlying() && !entity->isNpcFish();
}

void Border::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	IEntity::onPreSolve(contact, entity, oldManifold);
	if (!entity->isPlayer())
		return;

	Player* player = static_cast<Player*>(entity);
	if (_borderType == BorderType::TOP) {
		b2Vec2 v = player->getLinearVelocity();
		v.y = 0.4f;
		player->setLinearVelocity(v);
		contact->SetEnabled(false);
	}

	if (!_crashOnTouch)
		return;
	if (_map.isDone())
		return;

	player->setCrashed(CRASH_MAP_FAILED);
}

}
