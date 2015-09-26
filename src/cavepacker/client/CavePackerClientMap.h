#pragma once

#include "client/ClientMap.h"
#include <vector>

namespace cavepacker {

class CavePackerClientMap: public ClientMap {
private:
	SpritePtr _deadlockOverlay;
	mutable RenderTarget* _target;
	mutable std::size_t _targetEnts;
	void renderLayer (int x, int y, Layer layer) const override;

public:
	CavePackerClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	virtual void onWindowResize () override;

	int getWaterSurface() const override { return 0; }
	int getWaterGround() const override { return 0; }

	void undo ();

	void setDeadlocks(const std::vector<int>& deadlocks);

	void start () override;
	virtual void update (uint32_t deltaTime) override;
};

}
