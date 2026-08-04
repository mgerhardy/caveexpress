#pragma once

#include "caveexpress/server/entities/MapTile.h"
#include <string>

namespace caveexpress {

/**
 * @brief Wall-recessed barrier. Protrusion 0 = open (into background), maxOpenAmount = closed.
 * Collision is disabled when protrusion falls below a small threshold.
 */
class Gate: public MapTile {
private:
	std::string _linkId;
	float _maxOpenAmount;
	float _protrusion;
	float _targetProtrusion;
	bool _openRequested;

	void applyCollisionState ();
	void sendState () const;

public:
	Gate (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const std::string& linkId,
			float openAmount);
	virtual ~Gate ();

	const std::string& getLinkId () const { return _linkId; }
	void setLinkId (const std::string& linkId) { _linkId = linkId; }

	float getProtrusion () const { return _protrusion; }
	float getMaxOpenAmount () const { return _maxOpenAmount; }
	uint8_t getProtrusionByte () const;

	/** @param open true opens the gate (retract into wall) */
	void setOpen (bool open);
	bool isOpenRequested () const { return _openRequested; }

	void createBody () override;
	void update (uint32_t deltaTime) override;
	bool shouldCollide (const IEntity* entity) const override;
};

}
