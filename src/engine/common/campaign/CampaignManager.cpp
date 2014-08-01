#include "CampaignManager.h"
#include "Campaign.h"
#include "engine/common/Logger.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/MapManager.h"
#include "engine/common/Payment.h"
#include "engine/common/Commands.h"
#include "engine/common/FileSystem.h"
#include "engine/common/LUA.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/campaign/persister/IGameStatePersister.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/System.h"
#include "engine/common/Commands.h"

namespace {
CampaignManager *_mgr;
}

struct isEqual {
	const std::string& _campaignId;
	isEqual (const std::string& campaignId) : _campaignId(campaignId) {}
	bool operator() (const CampaignPtr& l) { return l->getId() == _campaignId; }
};

CampaignManager::CampaignManager (IGameStatePersister *persister, const IMapManager& mapManager) :
		_activeCampaign(), _persister(persister), _mapManager(mapManager), _completed(false)
{
	Commands.registerCommand(CMD_UNLOCK, bind(CampaignManager, unlock));
}

CampaignManager::~CampaignManager ()
{
	_campaigns.clear();
}

void CampaignManager::unlock ()
{
#ifdef DEBUG
	for (CampaignsMap::iterator i = _campaigns.begin(); i != _campaigns.end(); ++i) {
		CampaignPtr& c = *i;
		c->unlock();
		while (c->unlockNextMap(false)) {
		}
		c->saveProgress();
		c->loadProgress();
	}
#endif
}

void CampaignManager::init ()
{
	ExecutionTime e("Campaign loading");
	LUA lua;

	_mgr = this;

	luaL_Reg funcs[] = {
		{ "new", luaCreateCampaign },
		{ "addMap", luaAddMap },
		{ "addMaps", luaAddMaps },
		{ "unlockMap", luaUnlockMap },
		{ "unlock", luaUnlock },
		{ "setSetting", luaSetSetting },
		{ "loadProgress", luaLoadProgress },
		{ nullptr, nullptr }
	};

	lua.reg("Campaign", funcs);

	const std::string& campaignsPath = FS.getCampaignsDir();
	const DirectoryEntries& campaigns = FS.listDirectory(campaignsPath);
	int campaignsLoaded = 0;
	for (DirectoryEntries::const_iterator i = campaigns.begin(); i != campaigns.end(); ++i) {
		const std::string filename = campaignsPath + *i;
		if (!FS.hasExtension(filename, "lua"))
			continue;
		if (!lua.load(filename)) {
			error(LOG_CAMPAIGN, "failed to load campaign " + filename);
			continue;
		}
		++campaignsLoaded;
	}
	debug(LOG_CAMPAIGN, String::format("found %i campaign scripts", campaignsLoaded));

	if (System.hasItem(PAYMENT_UNLOCKALL)) {
		for (CampaignsMap::iterator i = _campaigns.begin(); i != _campaigns.end(); ++i) {
			CampaignPtr& c = *i;
			c->unlock();
			while (c->unlockNextMap(false)) {
			}
		}
	}

	_completed = isCompleted();
	const std::string completed = _completed ? "true" : "false";
	info(LOG_CAMPAIGN, "completed: " + completed);
}

int CampaignManager::luaCreateCampaign (lua_State *l)
{
	const std::string id = luaL_checkstring(l, 1);
	CampaignsMap::const_iterator i = std::find_if(_mgr->_campaigns.begin(), _mgr->_campaigns.end(), isEqual(id));
	if (i != _mgr->_campaigns.end()) {
		LUA::returnError(l, "Campaign " + id + " already exists");
		return 0;
	}
	Campaign ** udata = LUA::newUserdata<Campaign>(l, "Campaign");
	*udata = new Campaign(id, _mgr->_persister);
	info(LOG_CAMPAIGN, "create campaign " + id);
	_mgr->_campaigns.push_back(CampaignPtr(*udata));
	return 1;
}

int CampaignManager::luaUnlockMap (lua_State *l)
{
	Campaign *ctx = _luaGetContext(l, 1);
	const std::string mapID = luaL_checkstring(l, 2);
	CampaignMap* map = ctx->getMapById(mapID);
	if (map)
		map->unlock();
	return 0;
}

int CampaignManager::luaUnlock (lua_State *l)
{
	Campaign *ctx = _luaGetContext(l, 1);
	ctx->unlock();
	CampaignMapPtr map = ctx->getMap(1);
	if (map) {
		map->initialUnlock();
	}
	return 0;
}

int CampaignManager::luaAddMap (lua_State *l)
{
	Campaign *ctx = _luaGetContext(l, 1);
	const std::string mapID = luaL_checkstring(l, 2);
	const std::string name = _mgr->_mapManager.getMapTitle(mapID);
	ctx->addMap(mapID, name);
	return 0;
}

int CampaignManager::luaAddMaps (lua_State *l)
{
	Campaign *ctx = _luaGetContext(l, 1);
	const std::string mapWildcard = luaL_checkstring(l, 2);
	const IMapManager::Maps& maps = _mgr->_mapManager.getMapsByWildcard(mapWildcard);
	int cnt = 0;
	for (IMapManager::Maps::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		++cnt;
		const std::string name = _mgr->_mapManager.getMapTitle(i->first);
		ctx->addMap(i->first, name);
	}
	info(LOG_CAMPAIGN, String::format("%i maps added to campaign %s", cnt, ctx->getId().c_str()));
	return 0;
}

int CampaignManager::luaSetSetting (lua_State *l)
{
	Campaign *ctx = _luaGetContext(l, 1);
	const std::string key = luaL_checkstring(l, 2);
	const std::string value = luaL_checkstring(l, 3);
	ctx->setSetting(key, value);
	return 0;
}

int CampaignManager::luaLoadProgress (lua_State *l)
{
	Campaign *ctx = _luaGetContext(l, 1);
	ctx->loadProgress();
	return 0;
}

Campaign* CampaignManager::_luaGetContext (lua_State * l, int n)
{
	return LUA::getUserData<Campaign>(l, n, "Campaign");
}

CampaignPtr CampaignManager::getCampaign (const std::string& campaignId) const
{
	CampaignsMap::const_iterator i = std::find_if(_campaigns.begin(), _campaigns.end(), isEqual(campaignId));
	if (i == _campaigns.end()) {
		error(LOG_CAMPAIGN, "campaign with id " + campaignId + " was not found");
		return CampaignPtr();
	}

	return *i;
}

void CampaignManager::visitCampaigns (ICampaignVisitor *visitor)
{
	for (CampaignsMap::iterator i = _campaigns.begin(); i != _campaigns.end(); ++i) {
		visitor->visitCampaign(*i);
	}
}

CampaignPtr CampaignManager::activateCampaign (const std::string& campaignId) const
{
	_activeCampaign = getCampaign(campaignId);
	if (_activeCampaign) {
		if (!_activeCampaign->loadProgress())
			info(LOG_CAMPAIGN, "Could not load progress for " + campaignId);
		else
			info(LOG_CAMPAIGN, "Loaded progress for " + campaignId);
	} else {
		error(LOG_CLIENT, "could not get campaign with id " + campaignId);
	}
	return _activeCampaign;
}

void CampaignManager::notifyCampaignUnlock () const
{
	for (Listeners::const_iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onCampaignUnlock(_activeCampaign.get());
	}
}

bool CampaignManager::resetAllSavedData ()
{
	info(LOG_CAMPAIGN, "reset campaign progress");
	for (CampaignsMap::iterator i = _campaigns.begin(); i != _campaigns.end(); ++i) {
		const CampaignPtr& c = *i;
		if (!c->isUnlocked())
			continue;
		if (!c->reset(false))
			return false;
	}
	_campaigns.clear();
	_persister->reset();
	_activeCampaign = CampaignPtr();
	_completed = false;
	_lastPlayedMap = "";

	init();

	return true;
}

bool CampaignManager::resetActiveCampaign ()
{
	if (!_activeCampaign)
		return false;
	return _activeCampaign->reset();
}

bool CampaignManager::saveActiveCampaign ()
{
	if (!_activeCampaign)
		return false;
	return _activeCampaign->saveProgress();
}

CampaignPtr CampaignManager::getAutoActiveCampaign () const
{
	getActiveCampaign();
	while (_activeCampaign && !_activeCampaign->hasMoreMaps()) {
		info(LOG_CAMPAIGN, "try to activate the next campaign");
		activateNextCampaign();
	}
	return _activeCampaign;
}

CampaignPtr CampaignManager::getActiveCampaign () const
{
	if (!_activeCampaign) {
		const std::string campaign = _persister->getActiveCampaign();
		activateCampaign(campaign);
	}
	return _activeCampaign;
}

bool CampaignManager::activateNextCampaign () const
{
	_activeCampaign = CampaignPtr();
	for (CampaignsMap::const_iterator i = _campaigns.begin(); i != _campaigns.end(); ++i) {
		CampaignPtr c = *i;
		if (!c->hasMoreMaps()) {
			info(LOG_CLIENT, c->getId() + " has no more maps");
			continue;
		}
		activateCampaign(c->getId());
		if (_activeCampaign->unlock()) {
			notifyCampaignUnlock();
		}
		return true;
	}
	info(LOG_CAMPAIGN, "no more campaigns to unlock");
	return false;
}

bool CampaignManager::updateMapValues (const std::string& mapname, uint32_t finishPoints, uint32_t time, uint8_t stars, bool lowerPointsAreBetter)
{
	_lastPlayedMap = mapname;

	info(LOG_CAMPAIGN, String::format("map values for %s: %u points, time: %us and %i stars",
			_lastPlayedMap.c_str(), finishPoints, time, (int)stars));

	if (!_activeCampaign) {
		error(LOG_CAMPAIGN, "no active campaign");
		return false;
	}

	CampaignMap* map = _activeCampaign->getMapById(mapname);
	if (map == nullptr) {
		error(LOG_CAMPAIGN, "no map with the name " + mapname + " in the current active campaign " + _activeCampaign->getId());
		return false;
	}

	const bool alreadyPlayed = map->getTime() > 0;
	if (map->getStars() < stars)
		map->setStars(stars);
	map->setTime(time);
	if (lowerPointsAreBetter) {
		const int p = map->getFinishPoints();
		if (p <= 0 || p > finishPoints)
			map->setFinishPoints(finishPoints);
	} else {
		if (map->getFinishPoints() < finishPoints)
			map->setFinishPoints(finishPoints);
	}

	if (alreadyPlayed) {
		_activeCampaign->saveProgress();
		return true;
	}

	_activeCampaign->unlockNextMap();
	return true;
}

void CampaignManager::addListener (ICampaignManagerListener *listener)
{
	_listeners.push_back(listener);
}

void CampaignManager::removeListener (ICampaignManagerListener *listener)
{
	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		if (*i != listener)
			continue;
		_listeners.erase(i);
		break;
	}
}

void CampaignManager::reset ()
{
	_campaigns.clear();
	init();
}

bool CampaignManager::isNewlyCompleted ()
{
	if (_completed)
		return false;
	_completed = isCompleted();
	return _completed;
}

bool CampaignManager::isCompleted () const
{
	const CampaignPtr& campaign = getAutoActiveCampaign();
	if (campaign) {
		if (!campaign->isUnlocked())
			return false;
		const std::string& nextMap = campaign->getNextMap();
		const bool completed = nextMap.empty();
		return completed;
	}
	error(LOG_CAMPAIGN, "could not find any active campaign");
	return true;
}

bool CampaignManager::continuePlay ()
{
	const CampaignPtr& c = getAutoActiveCampaign();
	if (!c) {
		error(LOG_CAMPAIGN, "could not find any active campaign");
		return false;
	}
	const std::string& map = c->getNextMap();
	if (map.empty()) {
		error(LOG_CAMPAIGN, "empty map name in campaign " + c->getId());
		return false;
	}

	startMap(map);

	return true;
}

bool CampaignManager::firstMap () const
{
	const CampaignPtr& c = getAutoActiveCampaign();
	if (!c) {
		error(LOG_CAMPAIGN, "could not find any active campaign");
		return false;
	}
	return c->firstMap();
}

void CampaignManager::startMap (const std::string& map)
{
	info(LOG_CAMPAIGN, "start map " + map);
	Commands.executeCommandLine(CMD_MAP_START " " + map);
	_lastPlayedMap = map;
}

bool CampaignManager::replay ()
{
	if (_lastPlayedMap.empty())
		return false;
	info(LOG_CAMPAIGN, "replay map " + _lastPlayedMap);
	Commands.executeCommandLine(CMD_MAP_START " " + _lastPlayedMap);
	return true;
}
