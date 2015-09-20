#pragma once

#include "client/ClientMap.h"
#include <vector>

namespace cavepacker {

class CavePackerClientMap: public ClientMap {
private:
	std::vector<int> _deadlocks;

public:
	CavePackerClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	int getWaterSurface() const override { return 0; }
	int getWaterGround() const override { return 0; }

	void undo ();

	void clearDeadlocks();
	void addDeadlock(int index);

	void start () override;
	virtual void update (uint32_t deltaTime) override;
};

}
