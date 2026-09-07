#pragma once

#include "caveexpress/server/map/Map.h"
#include "common/String.h"

namespace caveexpress {

/**
 * Test-only Map that can host a Box2D world without loading a Lua campaign map.
 */
class PhysicsSandboxMap: public Map {
public:
	bool initPhysicsSandbox (int width, int height, float gravity = 9.81f, float waterHeight = 1.0f)
	{
		if (_frontend == nullptr || _serviceProvider == nullptr)
			return false;
		if (width < 4 || height < 4)
			return false;

		resetCurrentMap();
		_name = "physics-sandbox";
		_title = "physics-sandbox";
		_width = width;
		_height = height;
		_gravity = gravity;
		_waterHeight = waterHeight;
		_waterChangeSpeed = 0.0f;
		_flyingSpeedX = 1.0f;
		_warmupPhase = 0;
		_activateflyingNPC = false;
		_activateFishNPC = false;
		// isDone() is true when both transfer limits are 0, and setCrashed()
		// refuses to run on a finished map. Keep a dummy package goal so crash
		// contacts in the sandbox actually apply.
		_transferedPackageLimit = 1;
		_mapRunning = true;
		initPhysics();
		return _world != nullptr;
	}

	Player* spawnPlayerAt (float x, float y)
	{
		if (!_mapRunning || _world == nullptr)
			return nullptr;

		Player* player = new Player(*this, 1);
		player->setLives(3);
		_startPositions.clear();
		_startPositions.push_back({ string::toString(x), string::toString(y) });
		if (!spawnPlayer(player)) {
			delete player;
			return nullptr;
		}
		return player;
	}
};

}
