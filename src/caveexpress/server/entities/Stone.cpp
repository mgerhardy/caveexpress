#include "Stone.h"
#include "caveexpress/server/entities/Tree.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/npcs/NPCAggressive.h"
#include "caveexpress/server/entities/npcs/NPCFlying.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/shared/CaveExpressSoundType.h"

namespace caveexpress {

Stone::Stone (Map& map, gridCoord x, gridCoord y, const IEntity *creator) :
		CollectableEntity(EntityTypes::STONE, map, creator), _x(x), _y(y)
{
	setAnimationType(Animations::ANIMATION_IDLE);
}

Stone::~Stone ()
{
}

bool Stone::shouldCollide (const IEntity *entity) const
{
	if (CollectableEntity::shouldCollide(entity))
		return true;
	return entity->isStone() || entity->isPackage() || entity->isNpcBlowing() || entity->isNpcAggressive();
}

void Stone::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	CollectableEntity::onPreSolve(contact, entity, oldManifold);
	if (entity->isSolid() || entity->isStone()) {
		setLinearDamping(3.0f);
		setAngularDamping(1.0);
	} else if (entity->isBorder()) {
		setLinearDamping(4.0f);
		setAngularDamping(1.0);
	} else if (entity->isNpc()) {
		// if the stone is in rest, it does not have any impact on the npcs
		const Direction direction = getVelocityDirection(this, 3.0f);
		NPC *npc = assert_cast<NPC*, IEntity*>(entity);
		if (direction & DIRECTION_DOWN) {
			if (npc->isNpcBlowing() || npc->isNpcAttacking()) {
				npc->setDazed(_creator);
				contact->SetEnabled(false);
			} else if (entity->isNpcFlying() || entity->isNpcFish()) {
				npc->setDying(_creator);
			}
		} else {
			contact->SetEnabled(false);
		}
	} else if (entity->isPackage()) {
		// if the stone is in rest, it does not have any impact on the packages
		const Direction direction = getVelocityDirection(this, 3.0f);
		if (direction & DIRECTION_DOWN) {
			Package *package = assert_cast<Package*, IEntity*>(entity);
			package->setDestroyed(true);
		}
	}
}

void Stone::onContact (b2Contact* contact, IEntity* entity)
{
	CollectableEntity::onContact(contact, entity);
	if (!entity->isSolid() && !entity->isStone() && !entity->isPackage())
		return;
	if (isImpactVelocityMoreThan(contact, 1.5f))
		_map.sendSound(getVisMask(), SoundTypes::SOUND_STONE_COLLIDE, getPos());
}

void Stone::endContact (b2Contact* contact, IEntity* entity)
{
	IEntity::endContact(contact, entity);
	if (entity->isSolid() || entity->isStone() || entity->isBorder()) {
		setLinearDamping(0.0f);
		setAngularDamping(0.0);
	}
}

void Stone::createBody ()
{
	b2FixtureDef fd;
	fd.userData = nullptr;
	fd.density = DENSITY_STONE;
	fd.friction = 1.0f;
	fd.restitution = 0.01f;

	b2BodyDef bd;
	bd.userData = nullptr;
	bd.position.Set(_x, _y);
	bd.type = b2_dynamicBody;

	b2Body* body = _map.addToWorld(fd, bd, this);
	_map.addEntity(this);
	const b2Vec2 v(0.5f, _map.getGravity());
	body->SetLinearVelocity(v);
}

}
