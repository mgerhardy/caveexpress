#pragma once

#include "caveexpress/server/entities/npcs/INPCCave.h"

namespace caveexpress {

// forward decl
class CaveMapTile;
class Package;

/**
 * @brief This npc will put a package in front of its cave
 * @sa Package
 */
class NPCPackage : public INPCCave {
private:
	/** When true, idle-away-from-cave automatically dumps a package and walks home. */
	bool _autoLeavePackage;

public:
	NPCPackage (CaveMapTile *cave, const EntityType& type);
	virtual ~NPCPackage ();

	void setAutoLeavePackage (bool enabled);
	bool isAutoLeavePackage () const;

	/**
	 * @brief Spawn a package in front of the NPC without changing movement.
	 * @return the spawned package (never null)
	 */
	Package* dropPackage ();

	/**
	 * @brief Drop a package and start walking back to the cave.
	 * @return the spawned package (never null)
	 */
	Package* leavePackage ();

	// NPC
	void onSpawn () override;
	void setIdle () override;
	void update (uint32_t deltaTime) override;
	bool shouldCollide (const IEntity* entity) const override;
};

inline void NPCPackage::setAutoLeavePackage (bool enabled)
{
	_autoLeavePackage = enabled;
}

inline bool NPCPackage::isAutoLeavePackage () const
{
	return _autoLeavePackage;
}

}
