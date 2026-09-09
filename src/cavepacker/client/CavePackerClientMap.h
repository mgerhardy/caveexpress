#pragma once

#include "client/ClientMap.h"
#include <vector>

namespace cavepacker {

class CavePackerClientMap: public ClientMap {
private:
	SpritePtr _deadlockOverlay;
	ConfigVarPtr _moveLerpMillis;

public:
	CavePackerClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	int getWaterSurface() const override { return 0; }
	int getWaterGround() const override { return 0; }
	float getWind() const override { return 0.f; }

	uint32_t getEntityMoveLerpMillis () const override;

	void undo ();

	void setDeadlocks(const std::vector<int>& deadlocks);

	virtual void update (uint32_t deltaTime) override;
};

}
