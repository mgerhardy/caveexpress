#include "NPCPackage.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/CaveMapTile.h"

namespace caveexpress {

NPCPackage::NPCPackage (CaveMapTile *cave, const EntityType& type) :
		INPCCave(cave, type, true), _autoLeavePackage(true)
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
	// Packages are dumped onto the walkway; never block the cave NPC path.
	if (entity->isPackage())
		return false;
	return entity->isSolid();
}

void NPCPackage::setIdle ()
{
	NPC::setIdle();
}

void NPCPackage::update (uint32_t deltaTime)
{
	INPCCave::update(deltaTime);

	if (_autoLeavePackage) {
		if (isIdle() && !returnToInitialPosition())
			leavePackage();

		if (getCave()->moveBackIntoCave()) {
			Log::info(LOG_GAMEIMPL, "npc %i moved back into cave, remove from world", getID());
			_remove = true;
		}
		return;
	}

	// Script-driven dumpers: never auto-walk home on idle (that aborted multi-drop cutscenes).
	// Despawn only once the script has walked us back and we are idle at the cave.
	if (!isIdle())
		return;
	static const float gap = 0.1f;
	const float xPos = getPos().x;
	if (!Between(xPos, _initialPosition.x - gap, _initialPosition.x + gap))
		return;
	if (getCave() != nullptr && getCave()->getNPC() == this)
		getCave()->setNPC(nullptr);
	Log::info(LOG_GAMEIMPL, "npc %i scripted return complete, remove from world", getID());
	_remove = true;
}

Package* NPCPackage::dropPackage ()
{
	const PhysicsVec2& pos = getPos();
	const float offsetX = _lastDirectionRight ? 0.35f : -0.35f;
	Package* package = new Package(_map, pos.x + offsetX, pos.y);
	package->createBody();
	return package;
}

Package* NPCPackage::leavePackage ()
{
	Package* package = dropPackage();
	returnToInitialPosition();
	return package;
}

}
