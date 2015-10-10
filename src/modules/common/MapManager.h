#pragma once

#include "common/NonCopyable.h"
#include "common/LUA.h"
#include <string>
#include <map>

class MapData {
protected:
	std::string _id;
	std::string _name;
	int _startPositions;
public:
	MapData (const std::string& id, const std::string& name, int startPositions) :
			_id(id), _name(name), _startPositions(startPositions)
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

	inline int getStartPositions () const
	{
		return _startPositions;
	}
};

class IMapManager: public NonCopyable {
public:
	typedef std::map<std::string, MapData*> Maps;

protected:
	typedef Maps::const_iterator MapsConstIter;
	typedef Maps::iterator MapsIter;
	Maps _maps;
	std::string _extension;

	void listMaps ();

	virtual std::string getName (const std::string& filename, const std::string& id) {
		return id;
	}

	virtual int getStartPositions (const std::string& filename) {
		// MAX_CLIENTS
		return 4;
	}

public:
	explicit IMapManager (const std::string& extension);
	virtual ~IMapManager ();

	void init ();

	virtual void loadMaps ();

	Maps getMapsByWildcard (const std::string& wildcard) const;

	const std::string& getMapTitle (const std::string& mapId) const;
	int getMapStartPositions (const std::string& mapId) const;

	inline const Maps& getMaps () const
	{
		return _maps;
	}
};

class LUAMapManager: public IMapManager {
protected:
	LUA _lua;
public:
	LUAMapManager() : IMapManager("lua") {}

	std::string getName (const std::string& filename, const std::string& id) override {
		_lua.close();
		_lua.init();
		if (!_lua.load(filename)) {
			Log::error(LOG_COMMON, "could not load map from %s", filename.c_str());
			return id;
		}
		_lua.execute("getName", 1);
		const std::string& name = _lua.getStringFromStack();
		return name;
	}
};

class FileMapManager: public IMapManager {
public:
	explicit FileMapManager (const std::string& extension) : IMapManager(extension) {}
};
