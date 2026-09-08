#pragma once

#include "client/ClientMap.h"
#include "service/ServiceProvider.h"
#include "network/INetwork.h"

namespace caveexpress {

class CaveExpressClientMap: public ClientMap {
private:
	using Super = ClientMap;
	float _waterHeight = 0.0f;
	float _wind = 0.0f;
	mutable RenderTarget* _target = nullptr;
	uint16_t _spectateEntityId = 0;
	uint16_t _appliedSpectateHudEntityId = 0;
	bool _appliedSpectateHudValid = false;
	struct SpectatePlayerHud {
		uint16_t hitpoints = 0;
		uint8_t lives = 0;
		uint8_t targetCave = 0;
		uint8_t collectedTypeId = 0;
		bool operator== (const SpectatePlayerHud& o) const
		{
			return hitpoints == o.hitpoints && lives == o.lives && targetCave == o.targetCave
					&& collectedTypeId == o.collectedTypeId;
		}
	};
	SpectatePlayerHud _appliedSpectateHud;
	std::unordered_map<uint16_t, SpectatePlayerHud> _playerHud;

	void collectSpectateTargets (std::vector<uint16_t>& ids) const;
	void renderWater (int x, int y) const;
	void renderLavaHeat (int x, int y) const;
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
	ClientEntity* getSpectateTarget () const override;
	uint16_t cycleSpectateTarget (int dir) override;
	void storePlayerHud (uint16_t entityId, uint16_t hitpoints, uint8_t lives, uint8_t targetCave, uint8_t collectedTypeId);
	void applyFollowedPlayerHudIfChanged () override;
	void init (uint16_t playerID) override;
	bool keepSessionOnMatchEnd () const override { return _serviceProvider.getNetwork().isMultiplayer(); }
	void handleWaterImpact (float x, float force);
	void setWaterHeight (float height);
	// the water height in physic units
	float getWaterHeight () const;

	void renderBegin (int x, int y) const override;
	void renderEnd (int x, int y) const override;
	void renderLayer (int x, int y, Layer layer) const override;
	bool acceptSpriteLight (const ClientEntityPtr& e, const SpriteDefPtr& def) const override;
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
