#pragma once

#include "engine/common/NonCopyable.h"
#include <string>
#include <map>

class MapData {
protected:
	std::string _id;
	std::string _name;
public:
	MapData (const std::string& id, const std::string& name) :
			_id(id), _name(name)
	{
	}

	virtual ~MapData ()
	{
	}

	bool operator < (const MapData& rhs) const
	{
		return _id < rhs._id;
	}

	inline const std::string& getId () const
	{
		return _id;
	}

	inline const std::string& getName () const
	{
		return _name;
	}
};

class MapManager: public NonCopyable {
public:
	typedef std::map<std::string, MapData*> Maps;

private:
	typedef Maps::const_iterator MapsConstIter;
	typedef Maps::iterator MapsIter;
	Maps _maps;

	void listMaps ();
public:
	MapManager ();
	virtual ~MapManager ();

	void init ();
	void loadMaps ();

	Maps getMapsByWildcard (const std::string& wildcard) const;

	const std::string getMapTitle (const std::string& mapId) const;

	inline const Maps& getMaps () const
	{
		return _maps;
	}
};
