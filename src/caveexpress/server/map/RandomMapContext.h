#pragma once

#include <caveexpress/shared/CaveExpressMapContext.h>
#include "common/ThemeType.h"
#include "common/IMap.h"

namespace caveexpress {

/**
 * Thin facade over RandomMapGenerator for random-* maps and the editor.
 * All themes generate: rock/ice use package targets when present; jungle/desert
 * use NPC-transfer mode when package-target sprites are missing.
 */
class RandomMapContext: public CaveExpressMapContext {
private:
	unsigned int _caves;
	unsigned int _mapWidth;
	unsigned int _mapHeight;
	float _waterHeight;
	unsigned int _seedOverride; // 0 = use rand() / settings

public:
	RandomMapContext (const std::string& name, const ThemeType& theme, unsigned int width, unsigned int height);
	virtual ~RandomMapContext () = default;

	void setSettings (const IMap::SettingsMap& settings);
	void setFlyingNPC (bool flyingNPC);
	void setWind (float wind);
	void setCaves (unsigned int caves);
	void setWaterParameters (float waterHeight, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay);
	void setSeed (unsigned int seed);

	bool load (bool skipErrors) override;
};

}
