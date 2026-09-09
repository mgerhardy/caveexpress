#include "Platform.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/CaveMapTile.h"

namespace caveexpress {

Platform::Platform (Map& map) :
		IEntity(EntityTypes::PLATFORM, map), _caveTile(nullptr)
{
}

Platform::~Platform ()
{
}

bool Platform::shouldCollide (const IEntity* entity) const
{
	if (entity->isNpc()) {
		const NPC *npc = assert_cast<const NPC*, const IEntity*>(entity);
		return !npc->isFalling() && !npc->isDying();
	}
	return entity->isPlayer();
}

SpriteDefPtr Platform::getSpriteDef () const
{
	return SpriteDefPtr();
}

void Platform::onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold)
{
	IEntity::onPreSolve(contact, entity, oldManifold);
	if (!entity->isPlayer())
		return;

	Player *player = assert_cast<Player*, IEntity*>(entity);
	if (_caveTile == nullptr)
		return;

	// World-manifold normals are oriented by fixture A/B ordering. Compare
	// positions instead so below and side contacts can never become landings.
	if (player->getPos().y >= getPos().y - EPSILON)
		return;

	if (player->isLandedOn(_caveTile))
		return;

	const PhysicsManifold maniFold = contact.getManifold();
	if (maniFold.pointCount <= 0)
		return;

	player->setPlatform(this);
	Log::debug(LOG_GAMEIMPL, "player %s (%i) landed on cave %i", player->getName().c_str(), player->getID(), getID());
}

void Platform::endContact (PhysicsContact contact, IEntity* entity)
{
	IEntity::endContact(contact, entity);
	if (entity->isPlayer()) {
		Player *player = assert_cast<Player*, IEntity*>(entity);
		player->setPlatform(nullptr);
		Log::debug(LOG_GAMEIMPL, "player %s (%i) is no longer landed on cave %i", player->getName().c_str(), player->getID(), getID());
	}
}

}
