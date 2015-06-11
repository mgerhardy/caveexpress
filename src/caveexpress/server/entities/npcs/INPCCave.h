#pragma once

#include "caveexpress/server/entities/npcs/NPC.h"
#include "caveexpress/shared/CaveExpressEntityType.h"

// forward decl
class CaveMapTile;
class Map;

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

class INPCCave: public NPC {
protected:
	CaveMapTile *_cave;

	// used to measure the needed time to bring the npc to its target cave
	uint32_t _collectingTime;

	bool _deliverPackage;

public:
	INPCCave (CaveMapTile *cave, const EntityType& type, bool deliverPackage);

	virtual ~INPCCave ();

	void moveAwayFromCave ();
	bool isDeliverPackage () const;

	gridCoord getMaxWalkingLeft () const;
	gridCoord getMaxWalkingRight () const;

	CaveMapTile *getCave () const;

	// NPC
	void setPos (const b2Vec2& pos) override;
};

inline CaveMapTile *INPCCave::getCave () const
{
	return _cave;
}

inline bool INPCCave::isDeliverPackage () const
{
	return _deliverPackage;
}
