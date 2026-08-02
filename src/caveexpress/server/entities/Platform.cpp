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

	if (player->isLandedOn(_caveTile))
		return;

	PhysicsWorldManifold worldManifold;
	contact.getWorldManifold(worldManifold);
	const PhysicsVec2 worldNormal = worldManifold.normal;
	// not from above - then we don't care	// -1/sqrt(2)
	if (worldNormal.y >= -0.07 ||
		fabs(worldNormal.x) > fabs(worldNormal.y))
		return;

	const PhysicsManifold maniFold = contact.getManifold();
	if (maniFold.pointCount <= 0)
		return;
	const float normalImpulse = maniFold.points[0].normalImpulse;
	const float absNormalImpulse = fabs(normalImpulse);
	if (absNormalImpulse < EPSILON)
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
