#pragma once

#include "common/IMapContext.h"
#include <string>
#include "cavepacker/shared/SokobanTiles.h"

namespace cavepacker {

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
	bool save () const override;
};

}
