#pragma once

#include "caveexpress/server/entities/npcs/NPC.h"

// forward decl
class CaveMapTile;

class NPCPackage : public NPC {
protected:
	CaveMapTile *_cave;
public:
	NPCPackage (CaveMapTile *cave, const EntityType& type);
	virtual ~NPCPackage ();

	void leavePackage ();
	void moveAwayFromCave ();
	bool isDeliverPackage () const;

	gridCoord getMaxWalkingLeft () const;
	gridCoord getMaxWalkingRight () const;

	CaveMapTile *getCave () const;

	// NPC
	void onSpawn () override;
	void setIdle () override;
	void update (uint32_t deltaTime) override;
	bool shouldCollide (const IEntity* entity) const override;
};

inline CaveMapTile *NPCPackage::getCave () const
{
	return _cave;
}
