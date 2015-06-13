#include "NPCPackage.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/CaveMapTile.h"

namespace caveexpress {

NPCPackage::NPCPackage (CaveMapTile *cave, const EntityType& type) :
		INPCCave(cave, type, true)
{
	if (EntityTypes::isNpcGrandpa(_type))
		_initialWalkingSpeed = 0.9f;
}

NPCPackage::~NPCPackage ()
{
}

void NPCPackage::onSpawn ()
{
	moveAwayFromCave();
}

bool NPCPackage::shouldCollide (const IEntity* entity) const
{
	return entity->isSolid();
}

void NPCPackage::setIdle ()
{
	debug(LOG_SERVER, String::format("idle npc %i: %s", getID(), _type.name.c_str()));
	setState(NPCState::NPC_IDLE);
	setLinearVelocity(b2Vec2_zero);
	_idleTimer = 0;
}

void NPCPackage::update (uint32_t deltaTime)
{
	INPCCave::update(deltaTime);

	if (isIdle() && !returnToInitialPosition())
		leavePackage();

	if (getCave()->moveBackIntoCave()) {
		info(LOG_SERVER, String::format("npc %i moved back into cave, remove from world", getID()));
		_remove = true;
	}
}

void NPCPackage::leavePackage ()
{
	const b2Vec2& pos = getPos();
	Package* package = new Package(_map, pos.x, pos.y);
	package->createBody();
	returnToInitialPosition();
}

}
