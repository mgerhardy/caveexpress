#pragma once

#include "engine/client/ClientMap.h"

class CaveExpressClientMap: public ClientMap {
private:
	float _waterHeight;
	uint32_t _lastClickedOnPlayer;

	void renderWater (int x, int y) const;
	void couldNotFindEntity (const std::string& prefix, uint16_t id) const override;
	void resetCurrentMap () override;

public:
	CaveExpressClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	void setCaveState (uint16_t id, bool state);
	bool drop ();
	void handleWaterImpact (float x, float force);
	void setWaterHeight (float height);
	// the water height in physic units
	float getWaterHeight () const;

	int getWaterSurface () const override { return _y + _waterHeight * _scale; }
	int getWaterGround () const override { return getWaterSurface() + (_mapHeight - _waterHeight) * _scale; }

	bool secondFinger () override { return drop(); }
	bool playerClickedByFinger (bool up) override;

	void render (int x, int y) const override;
};

inline void CaveExpressClientMap::handleWaterImpact (float x, float force)
{
}

inline void CaveExpressClientMap::setWaterHeight (float height)
{
	_waterHeight = height;
}

inline float CaveExpressClientMap::getWaterHeight () const
{
	return _waterHeight;
}
