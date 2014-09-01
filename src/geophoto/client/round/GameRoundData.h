#pragma once

#include "client/round/Location.h"
#include "shared/FileSystem.h"
#include <vector>

typedef std::vector<Location> Locations;

class GameRoundData {
private:
	Locations _locations;
public:
	GameRoundData ()
	{
	}

	virtual ~GameRoundData ()
	{
	}

	void addLocation (Location& location);
	const Locations& getLocations () const;
};

inline void GameRoundData::addLocation (Location& location)
{
	FileSystem& fs = FS.get();

	const URI imageUrl(location.url);
	const URI thumbUrl(location.thumbUrl);

	FilePtr imagePtr = fs.getFile(imageUrl);
	FilePtr thumbPtr = fs.getFile(thumbUrl);

	fs.cache(imagePtr, fs.getPicsDir());
	fs.cache(thumbPtr, fs.getPicsDir());

	_locations.push_back(location);
}

inline const Locations& GameRoundData::getLocations () const
{
	return _locations;
}
