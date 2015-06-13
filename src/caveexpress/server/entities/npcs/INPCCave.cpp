#include "caveexpress/server/entities/npcs/INPCCave.h"
#include "caveexpress/server/entities/CaveMapTile.h"

namespace caveexpress {

INPCCave::INPCCave (CaveMapTile *cave, const EntityType& type, bool deliverPackage) :
		NPC(EntityTypes::isNpcCave(type) ? type : getNpcFriendlyType(), cave->getMap()), _cave(cave), _collectingTime(0), _deliverPackage(
				deliverPackage)
{
	const b2Vec2& caveSize = cave->getSize();
	const b2Vec2& npcSize = getSize();
	const float yDelta = (caveSize.y - npcSize.y) / 2.0f;
	b2Vec2 cavePos = cave->getPos();
	cavePos.y += yDelta;
	createBody(cavePos);
}

INPCCave::~INPCCave ()
{
}

void INPCCave::setPos (const b2Vec2& pos)
{
	// sanity check
	if (pos.x < getMaxWalkingLeft() || pos.x > getMaxWalkingRight())
		error(LOG_SERVER, "invalid position given");
	NPC::setPos(pos);
}

void INPCCave::moveAwayFromCave ()
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
	const gridCoord posX = getCave()->getPos().x;
	gridCoord start = leftTileX;
	gridCoord end = rightTileX;
	if (fabs(posX - leftTileX) > fabs(posX - rightTileX)) {
		end = posX - middle;
	} else {
		start = posX + middle;
	}
	gridCoord moveTargetX = randBetweenf(start, end);
	info(LOG_SERVER, String::format("moveTarget: %f, start: %f, end: %f, posX: %f", moveTargetX, start, end, posX));
	if (EntityTypes::isNpcGrandpa(_type)) {
		const gridCoord maxWalkingDistance = 3.0f;
		if (moveTargetX < posX - maxWalkingDistance)
			moveTargetX = posX - maxWalkingDistance;
		else if (moveTargetX > posX + maxWalkingDistance)
			moveTargetX = posX + maxWalkingDistance;
	}

	setMoving(moveTargetX);
}

gridCoord INPCCave::getMaxWalkingLeft () const
{
	return _cave->getPlatformStartGridX() + 0.2f;
}

gridCoord INPCCave::getMaxWalkingRight () const
{
	return _cave->getPlatformEndGridX() - 0.2f;
}

}
