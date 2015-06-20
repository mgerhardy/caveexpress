#pragma once

#include "client/ClientMap.h"

namespace caveexpress {

class CaveExpressClientMap: public ClientMap {
private:
	float _waterHeight;
	mutable RenderTarget* _target;

	void renderWater (int x, int y) const;
	void couldNotFindEntity (const std::string& prefix, uint16_t id) const override;
	void resetCurrentMap () override;

public:
	CaveExpressClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	void setCaveNumber (uint16_t id, uint8_t number);
	void setCaveState (uint16_t id, bool state);
	bool drop ();
	void start () override;
	void init (uint16_t playerID) override;
	void handleWaterImpact (float x, float force);
	void setWaterHeight (float height);
	// the water height in physic units
	float getWaterHeight () const;

	void renderBegin (int x, int y) const;
	void renderEnd (int x, int y) const;

	int getWaterSurface () const override { return (int)((_waterHeight + 0.00001f) * static_cast<float>(_scale)); }
	int getWaterGround () const override { return getWaterSurface() + _mapHeight - (int)((_waterHeight + 0.00001f) * static_cast<float>(_scale)); }

	bool secondFinger () override { return drop(); }
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

}
