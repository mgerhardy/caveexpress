#pragma once

#include "caveexpress/server/entities/MapTile.h"
#include <set>
#include <string>

namespace caveexpress {

class Gate;

/**
 * @brief Solid ground tile that opens a linked gate when weighted (player/stone, or package mass).
 * NPCs can walk across it without activating it.
 */
class PressurePlate: public MapTile {
private:
	std::string _linkId;
	float _requiredWeight;
	int _holdMs;
	Gate* _linkedGate;
	std::set<IEntity*> _contacts;
	uint32_t _holdUntil;
	bool _active;

	bool shouldActivate () const;
	void setActive (bool active);

public:
	PressurePlate (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const std::string& linkId,
			float requiredWeight, int holdMs);
	virtual ~PressurePlate ();

	const std::string& getLinkId () const { return _linkId; }
	void setLinkId (const std::string& linkId) { _linkId = linkId; }
	float getRequiredWeight () const { return _requiredWeight; }
	void setRequiredWeight (float w) { _requiredWeight = w; }
	int getHoldMs () const { return _holdMs; }
	void setHoldMs (int ms) { _holdMs = ms; }
	bool isActive () const { return _active; }
	Gate* getLinkedGate () const { return _linkedGate; }
	void setLinkedGate (Gate* gate) { _linkedGate = gate; }

	void update (uint32_t deltaTime) override;
	void onContact (PhysicsContact contact, IEntity* entity) override;
	void endContact (PhysicsContact contact, IEntity* entity) override;
	bool shouldCollide (const IEntity* entity) const override;
};

}
