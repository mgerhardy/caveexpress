#pragma once

#include "common/IMapContext.h"
#include <string>

namespace cavepacker {

namespace Sokoban {
const int WALL = '#';
const int PLAYER = '@';
const int PACKAGE = '$';
const int TARGET = '.';
const int GROUND = ' ';
const int PACKAGEONTARGET = '*';
const int PLAYERONTARGET = '+';
}

class SokobanMapContext : public IMapContext {
private:
	bool _playerSpawned;

	void addPlayer(int col, int row);
	void addWall(int col, int row);
	void addGround(int col, int row);
	void addPackage(int col, int row);
	void addTarget(int col, int row);
	void addTile(const std::string& tile, int col, int row);
	bool isEmpty(int col, int row) const;
public:
	explicit SokobanMapContext(const std::string& map);
	virtual ~SokobanMapContext();

	// IMapContext
	void onMapLoaded () override;
	bool load (bool skipErrors) override;
	bool save () const override { return false; }
};

}
