#pragma once

#include "engine/common/IMapContext.h"
#include <string>

class SokubanMapContext : public IMapContext {
private:
	void addPlayer(int col, int row);
	void addWall(int col, int row);
	void addGround(int col, int row);
	void addPackage(int col, int row);
	void addTarget(int col, int row);
	void addTile(const std::string& tile, int col, int row);
public:
	SokubanMapContext(const std::string& map);
	virtual ~SokubanMapContext();

	// IMapContext
	void onMapLoaded () override;
	bool load (bool skipErrors) override;
	bool save () const override { return false; }
};
