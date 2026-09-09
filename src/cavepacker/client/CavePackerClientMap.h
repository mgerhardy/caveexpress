#pragma once

#include "client/ClientMap.h"

namespace cavepacker {

class CavePackerClientMap: public ClientMap {
private:
	SpritePtr _deadlockOverlay;
	ConfigVarPtr _moveLerpMillis;
	bool isSpectateCandidate (const ClientEntity& entity) const override;

public:
	CavePackerClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	int getWaterSurface() const override { return 0; }
	int getWaterGround() const override { return 0; }
	float getWind() const override { return 0.f; }

	uint32_t getEntityMoveLerpMillis () const override;
	float getDefaultAmbientLight () const override { return 0.7f; }

	void undo ();

	void setDeadlocks(const std::vector<int>& deadlocks);

	virtual void update (uint32_t deltaTime) override;
};

}
