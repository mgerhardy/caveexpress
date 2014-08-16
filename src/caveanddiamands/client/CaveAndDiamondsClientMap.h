#pragma once

#include "engine/client/ClientMap.h"

class CaveAndDiamondsClientMap: public ClientMap {
public:
	CaveAndDiamondsClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	int getWaterSurface() const override { return 0; }
	int getWaterGround() const override { return 0; }

	void start () override;
};
