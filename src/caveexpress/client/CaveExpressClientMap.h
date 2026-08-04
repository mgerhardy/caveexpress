#pragma once

#include "client/ClientMap.h"

namespace caveexpress {

class CaveExpressClientMap: public ClientMap {
private:
	using Super = ClientMap;
	float _waterHeight = 0.0f;
	float _wind = 0.0f;
	mutable RenderTarget* _target = nullptr;

	void renderWater (int x, int y) const;
	SDL_Rect getWaterRect(int x, int y) const;
	void couldNotFindEntity (const std::string& prefix, uint16_t id) const override;
	void resetCurrentMap () override;
	void calcCaveSignOffset(const ClientEntityPtr &e, const SpritePtr &caveSignSprite, const SpritePtr &caveSprite,
							int &offsetX, int &offsetY);

public:
	CaveExpressClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	void setCaveNumber(uint16_t id, uint8_t number);
	void setCaveState (uint16_t id, bool state);
	void setGateState (uint16_t id, uint8_t openAmount);
	bool drop ();
	void start () override;
	void init (uint16_t playerID) override;
	void handleWaterImpact (float x, float force);
	void setWaterHeight (float height);
	// the water height in physic units
	float getWaterHeight () const;

	void renderBegin (int x, int y) const override;
	void renderEnd (int x, int y) const override;
	void renderLayer (int x, int y, Layer layer) const override;
	int renderCooldownDescription (uint32_t cooldownIndex, int x, int y, int w, int h) const override;
	void setSetting (const std::string& key, const std::string& value) override;

	int getWaterSurface () const override { return (int)((_waterHeight + 0.00001f) * static_cast<float>(_scaleGridToPixel)); }
	int getWaterGround () const override { return getWaterSurface() + _mapGridHeight - (int)((_waterHeight + 0.00001f) * static_cast<float>(_scaleGridToPixel)); }
	float getWind () const override {  return _wind;  }

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
