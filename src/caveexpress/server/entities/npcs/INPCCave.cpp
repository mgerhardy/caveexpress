#include "caveexpress/server/entities/npcs/INPCCave.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "common/Log.h"

namespace caveexpress {

INPCCave::INPCCave (CaveMapTile *cave, const EntityType& type, bool deliverPackage) :
		NPC(EntityTypes::isNpcCave(type) ? type : getNpcFriendlyType(), cave->getMap()), _cave(cave), _deliverPackage(
				deliverPackage)
{
	const PhysicsVec2& caveSize = cave->getSize();
	const PhysicsVec2& npcSize = getSize();
	const float yDelta = (caveSize.y - npcSize.y) / 2.0f;
	PhysicsVec2 cavePos = cave->getPos();
	cavePos.y += yDelta;
	createBody(cavePos);
}

INPCCave::~INPCCave ()
{
}

void INPCCave::setPos (const PhysicsVec2& pos)
{
	const gridCoord left = getMaxWalkingLeft();
	const gridCoord right = getMaxWalkingRight();
	if (pos.x < left || pos.x > right) {
		const int caveX = static_cast<int>(_cave->getGridX());
		const int caveY = static_cast<int>(_cave->getGridY());
		if (left > right)
			Log::error(LOG_GAMEIMPL,
					"cave villager %s cannot stand at cave %i (grid %i,%i) on map %s: landing platform is too short (walkable x %.2f..%.2f). Add ground tiles connected to the cave.",
					_type.name.c_str(), _cave->getCaveNumber(), caveX, caveY, _map.getName().c_str(), left, right);
		else
			Log::error(LOG_GAMEIMPL,
					"cave villager %s was placed at %.2f,%.2f outside cave %i platform x=%.2f..%.2f (cave grid %i,%i on map %s)",
					_type.name.c_str(), pos.x, pos.y, _cave->getCaveNumber(), left, right, caveX, caveY,
					_map.getName().c_str());
	}
	NPC::setPos(pos);
}

void INPCCave::moveAwayFromCave ()
{
	const gridCoord leftTileX = getMaxWalkingLeft();
	const gridCoord rightTileX = getMaxWalkingRight();
	// not possible - not enough space
	if (fequals(leftTileX, rightTileX, 0.02f)) {
		Log::error(LOG_GAMEIMPL,
				"cave villager %s cannot walk out of cave %i (grid %i,%i) on map %s: landing platform is too short (walkable x %.2f..%.2f). Add ground tiles connected to the cave.",
				_type.name.c_str(), _cave->getCaveNumber(), static_cast<int>(_cave->getGridX()),
				static_cast<int>(_cave->getGridY()), _map.getName().c_str(), leftTileX, rightTileX);
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
	Log::debug(LOG_GAMEIMPL, "moveTarget: %f, start: %f, end: %f, posX: %f", moveTargetX, start, end, posX);
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
