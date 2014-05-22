#include "NPCPackage.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/CaveMapTile.h"

namespace {
static inline const EntityType& getNpcFriendlyType ()
{
	const int index = rand() % 3;
	if (index == 0)
		return EntityTypes::NPC_FRIENDLY_GRANDPA;
	else if (index == 1)
		return EntityTypes::NPC_FRIENDLY_MAN;
	return EntityTypes::NPC_FRIENDLY_WOMAN;
}
}

NPCPackage::NPCPackage (CaveMapTile *cave, const EntityType& type) :
		NPC(EntityTypes::isNpcCave(type) ? type : getNpcFriendlyType(), cave->getMap()), _cave(cave)
{
	const b2Vec2& caveSize = cave->getSize();
	const b2Vec2& npcSize = getSize();
	const float yDelta = (caveSize.y - npcSize.y) / 2.0f;
	b2Vec2 cavePos = cave->getPos();
	cavePos.y += yDelta;
	createBody(cavePos);

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
	NPC::update(deltaTime);

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

void NPCPackage::moveAwayFromCave ()
{
	const gridCoord leftTileX = getMaxWalkingLeft();
	const gridCoord rightTileX = getMaxWalkingRight();
	// not possible - not enough space
	if (fequals(leftTileX, rightTileX, 0.02f)) {
		error(LOG_SERVER, "move away from cave is not possible, there is not enough space");
		// TODO: destroy the npc
		return;
	}

	static const gridSize middle = 0.5f;
	const gridCoord posX = _cave->getPos().x;
	gridCoord start = leftTileX;
	gridCoord end = rightTileX;
	if (fabs(posX - leftTileX) > fabs(posX - rightTileX)) {
		end = posX - middle;
	} else {
		start = posX + middle;
	}

	gridCoord moveTargetX = randBetweenf(start, end);
	if (EntityTypes::isNpcGrandpa(_type)) {
		const gridCoord maxWalkingDistance = 3.0f;
		if (moveTargetX < posX - maxWalkingDistance)
			moveTargetX = posX - maxWalkingDistance;
		else if (moveTargetX > posX + maxWalkingDistance)
			moveTargetX = posX + maxWalkingDistance;
	}
	debug(LOG_SERVER, String::format("moveTarget: %f, start: %f, end: %f, posX: %f", moveTargetX, start, end, posX));

	setMoving(moveTargetX);
}

gridCoord NPCPackage::getMaxWalkingLeft () const
{
	return _cave->getPlatformStartGridX() + 0.2f;
}

gridCoord NPCPackage::getMaxWalkingRight () const
{
	return _cave->getPlatformEndGridX() - 0.2f;
}
