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

void Platform::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	IEntity::onPreSolve(contact, entity, oldManifold);
	if (!entity->isPlayer())
		return;

	Player *player = assert_cast<Player*, IEntity*>(entity);
	if (_caveTile == nullptr)
		return;

	if (player->isLandedOn(_caveTile))
		return;

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);
	const b2Vec2 worldNormal = worldManifold.normal;
	// not from above - then we don't care
	// -1/sqrt(2)
	if (worldNormal.y >= -0.707)
		return;

	const b2Manifold *maniFold = contact->GetManifold();
	if (maniFold->pointCount < 2)
		return;

	const float32 normalImpulse = maniFold->points[0].normalImpulse;
	const float32 absNormalImpulse = fabs(normalImpulse);
	if (absNormalImpulse < EPSILON)
		return;

	for (int i = 0; i < maniFold->pointCount; i++) {
		// shut up compiler
		if (i == 0)
			continue;
		// the impulses must match - otherwise we are not in rest
		const float impulse = fabs(normalImpulse - maniFold->points[i].normalImpulse);
		if (impulse > 5.0f)
			return;
	}
	player->setPlatform(this);
	Log::debug(LOG_GAMEIMPL, "player %s (%i) landed on cave %i", player->getName().c_str(), player->getID(), getID());
}

void Platform::endContact (b2Contact* contact, IEntity* entity)
{
	IEntity::endContact(contact, entity);
	if (entity->isPlayer()) {
		Player *player = assert_cast<Player*, IEntity*>(entity);
		player->setPlatform(nullptr);
		Log::debug(LOG_GAMEIMPL, "player %s (%i) is no longer landed on cave %i", player->getName().c_str(), player->getID(), getID());
	}
}

}
