#include "CollectableEntity.h"
#include "caveexpress/server/entities/Player.h"

namespace caveexpress {

CollectableEntity::CollectableEntity (const EntityType& type, Map& map, const IEntity *creator) :
		IEntity(type, map), _collected(false), _creator(creator), _collectedTime(0), _collector(nullptr), _lastDropper(nullptr), _collectCount(0)
{
	setAnimationType(Animations::ANIMATION_IDLE);
	setState(EntityState::ENTITY_NORMAL);
}

void CollectableEntity::onContact (b2Contact* contact, IEntity* entity)
{
	IEntity::onContact(contact, entity);
	if (isDestroyed() || isCollected() || isRemove())
		return;
	if (entity->isPlayer()) {
		Player *player = assert_cast<Player*, IEntity*>(entity);
		if (player->collect(this)) {
			setCollected(true, player);
		}
	}
}

bool CollectableEntity::isRemove () const
{
	return isCollected() || IEntity::isRemove();
}

bool CollectableEntity::shouldCollide (const IEntity *entity) const
{
	if (IEntity::shouldCollide(entity))
		return true;

	// the entity must be active for some time to hit the creator entity (again)
	// this is needed so that a dropped stone or other item would not get collected again
	// too soon
	if (_creator == entity) {
		if (_time > 2000) {
			_creator = nullptr;
			return true;
		}
		return false;
	}

	return entity->isSolid() || entity->isWater() || entity->isPlayer() || entity->isCollectable();
}

}
