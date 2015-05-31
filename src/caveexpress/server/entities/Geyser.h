#pragma once

#include "caveexpress/server/entities/MapTile.h"

class WindModificator;

class Geyser: public MapTile {
private:
	WindModificator *_modificator;
	uint32_t _lastActivation;
	uint32_t _activeTime;
	uint32_t _initialGeyserDelay;

	void updateLastActivation ();
public:
	Geyser(Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, uint32_t initialGeyserDelay);
	virtual ~Geyser();

	void update (uint32_t deltaTime) override;
};
