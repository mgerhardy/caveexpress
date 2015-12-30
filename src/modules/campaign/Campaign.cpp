#include "Campaign.h"
#include "persister/IGameStatePersister.h"
#include "common/MapManager.h"
#include "common/Log.h"

Campaign::Campaign (const std::string& id, IGameStatePersister* persister) :
		_id(id), _currentMap(0), _persister(persister), _lives(INITIAL_LIVES)
{
}

Campaign::~Campaign ()
{
}

CampaignMapPtr Campaign::getMap (int index) const
{
	if (index < 1 || index > static_cast<int>(_maps.size()))
		return CampaignMapPtr();
	return _maps[index - 1];
}

CampaignMap* Campaign::getMapById (const std::string& mapname)
{
	for (Campaign::MapListIter i = _maps.begin(); i != _maps.end(); ++i) {
		if ((*i)->getId() == mapname) {
			return i->get();
		}
	}
	return nullptr;
}

void Campaign::resetMaps ()
{
	for (Campaign::MapListConstIter i = _maps.begin(); i != _maps.end(); ++i) {
		(*i)->reset();
	}
	_currentMap = 0;
}

bool Campaign::loadProgress ()
{
	if (!_persister->loadCampaign(this)) {
		return false;
	}
	Log::info(LOG_CAMPAIGN, "loaded campaign progress for: %s (lives: %i)", getId().c_str(), static_cast<int>(getLives()));
	_currentMap = 0;
	for (Campaign::MapListConstIter i = _maps.begin(); i != _maps.end(); ++i) {
		if (!(*i)->isLocked())
			++_currentMap;
		else
			break;
	}

	return true;
}

bool Campaign::unlock ()
{
	// if we already played this campaign, we will not unlock any other map
	if (isUnlocked()) {
		Log::info(LOG_CAMPAIGN, "no unlocking needed for campaign '%s'", getId().c_str());
		return false;
	}

	CampaignMapPtr map = getMap(1);
	if (!map) {
		Log::info(LOG_CAMPAIGN, "no map in campaign '%s'", getId().c_str());
		return false;
	}

	Log::info(LOG_CAMPAIGN, "unlocked campaign '%s'", getId().c_str());
	map->unlock();
	_currentMap = 1;
	return true;
}

bool Campaign::reset (bool unlockFirstMap)
{
	const bool alreadyUnlocked = isUnlocked();
	if (!alreadyUnlocked) {
		Log::error(LOG_CAMPAIGN, "could not reset campaign '%s' - not yet unlocked", getId().c_str());
		return false;
	}

	if (!_persister->resetCampaign(this)) {
		Log::error(LOG_CAMPAIGN, "failed to reset the campaign");
		return false;
	}

	resetMaps();
	if (unlockFirstMap)
		unlock();

	return true;
}

bool Campaign::saveProgress ()
{
	Log::info(LOG_CAMPAIGN, "save campaign progress for: %s", _id.c_str());
	return _persister->saveCampaign(this);
}

bool Campaign::firstMap () const
{
	return _currentMap == 1;
}

bool Campaign::isUnlocked () const
{
	return _currentMap != 0;
}

void Campaign::setLives (uint8_t lives)
{
	Log::debug(LOG_CAMPAIGN, "set lives: %i", static_cast<int>(getLives()));
	_lives = lives;
}

uint8_t Campaign::getLives () const
{
	return _lives;
}

void Campaign::setSetting (const std::string& key, const std::string& value)
{
	_settings[key] = value;
}

void Campaign::addMap (const std::string& id, const std::string &name)
{
	CampaignMapPtr map(new CampaignMap(id, name, true));
	_maps.push_back(map);
}

bool Campaign::unlockNextMap (bool shouldSaveProgress)
{
	CampaignMapPtr map = getMap(++_currentMap);
	if (!map) {
		_currentMap = 1;
		Log::info(LOG_CAMPAIGN, "no more maps to unlock");
		if (shouldSaveProgress)
			saveProgress();
		return false;
	}
	map->unlock();
	if (shouldSaveProgress)
		saveProgress();
	return true;
}

std::string Campaign::getNextMap () const
{
	CampaignMapPtr map = getMap(_currentMap);
	if (!map)
		return "";

	if (map->isLocked()) {
		Log::debug(LOG_CAMPAIGN, "next campaign map is still locked: %s", map->getId().c_str());
		return "";
	}

	Log::info(LOG_CAMPAIGN, "next campaign map %s", map->getId().c_str());
	return map->getId();
}

// will return a setting for a campaign from the map definition.
std::string Campaign::getSetting (const std::string& key) const
{
	SettingsMapConstIter iter = _settings.find(key);
	if (iter != _settings.end()) {
		return iter->second;
	}

	return "";
}

bool Campaign::hasMoreMaps () const
{
	if (_maps.empty())
		return false;
	const uint32_t time = (*_maps.rbegin())->getTime();
	Log::info(LOG_CAMPAIGN, "campaign %s last map time: %u", getId().c_str(), time);
	return time == 0;
}
