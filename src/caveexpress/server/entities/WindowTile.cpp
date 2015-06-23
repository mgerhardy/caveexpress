#include "WindowTile.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "common/Log.h"

namespace caveexpress {

WindowTile::WindowTile (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY) :
		MapTile(map, spriteID, gridX, gridY, EntityTypes::WINDOW), _lightState(DEFAULT_LIGHT_STATE), _cave(nullptr)
{
}

WindowTile::~WindowTile ()
{
}

void WindowTile::setLightState (bool lightState)
{
	const bool old = _lightState;
	_lightState = lightState;
	if (old == _lightState)
		return;

	GameEvent.sendLightState(getVisMask(), getID(), _lightState);
}

}
