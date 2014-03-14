#pragma once

#include "caveexpress/server/entities/MapTile.h"
#include <string>

// forward decl
class CaveMapTile;

// maptile without a physic body and a window state
class WindowTile: public MapTile {
private:
	bool _lightState;
	CaveMapTile* _cave;
public:
	WindowTile (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY);
	virtual ~WindowTile ();

	// returns true if the light of the window is activated, false otherwise
	bool getLightState () const;
	void setLightState (bool state);
	CaveMapTile* getCave () const;
};

inline bool WindowTile::getLightState () const
{
	return _lightState;
}

inline CaveMapTile* WindowTile::getCave () const
{
	return _cave;
}
