#include "Border.h"
#include "caveexpress/server/entities/npcs/NPCAggressive.h"
#include "caveexpress/server/map/Map.h"

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
	if (!_crashOnTouch)
		return;

	if (!entity->isPlayer())
		return;

	if (_map.isDone())
		return;

	Player* player = static_cast<Player*>(entity);
	player->setCrashed(CRASH_MAP_FAILED);
}
