#pragma once

#include "client/ClientMap.h"

namespace miniracer {

class MiniRacerClientMap: public ClientMap {
public:
	MiniRacerClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);
	int getWaterSurface() const override { return 0; }
	int getWaterGround() const override { return 0; }
};

}
