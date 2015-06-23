
#include "MapManager.h"
#include "common/Log.h"
#include "common/System.h"
#include "common/FileSystem.h"
#include "common/CommandSystem.h"
#include "common/Commands.h"

namespace {
const std::string EMPTY = "";
}

IMapManager::IMapManager (const std::string& extension) : _extension(extension)
{
}

IMapManager::~IMapManager ()
{
	for (MapsIter i = _maps.begin(); i != _maps.end(); ++i) {
		delete i->second;
	}
	Commands.removeCommand(CMD_LIST_MAPS);
}

const std::string& IMapManager::getMapTitle (const std::string& mapId) const
{
	MapsConstIter i = _maps.find(mapId);
	if (i == _maps.end())
		return EMPTY;

	return i->second->getName();
}

int IMapManager::getMapStartPositions (const std::string& mapId) const
{
	MapsConstIter i = _maps.find(mapId);
	if (i == _maps.end())
		return -1;

	return i->second->getStartPositions();
}

void IMapManager::init ()
{
	loadMaps();

	Commands.registerCommand(CMD_LIST_MAPS, bindFunction(IMapManager, listMaps));
}

void IMapManager::listMaps ()
{
	Log::info(LOG_SERVER, "Map list:");
	for (MapsConstIter i = _maps.begin(); i != _maps.end(); ++i) {
		Log::info(LOG_SERVER, " * %s", i->first.c_str());
	}
}

IMapManager::Maps IMapManager::getMapsByWildcard (const std::string& wildcard) const
{
	Maps maps;
	const String tmp(wildcard);
	for (MapsConstIter i = _maps.begin(); i != _maps.end(); ++i) {
		if (!tmp.matches(i->first))
			continue;
		maps[i->first] = i->second;
	}
	return maps;
}

void IMapManager::loadMaps ()
{
	_maps.clear();
	const std::string& mapsPath = FS.getMapsDir();
	const DirectoryEntries& maps = FS.listDirectory(mapsPath);
	for (DirectoryEntries::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		const std::string filename = mapsPath + *i;
		if (!FS.hasExtension(filename, _extension))
			continue;
		const int baseLength = i->size() - 4;
		const std::string& id = i->substr(0, baseLength);
		if (_maps.find(id) != _maps.end()) {
			Log::error(LOG_MAP, "map with id %s already exists", id.c_str());
			continue;
		}
		_maps[id] = new MapData(id, getName(filename, id), getStartPositions(filename));
	}

	Log::info(LOG_MAP, "loaded %i maps", static_cast<int>(_maps.size()));
}

