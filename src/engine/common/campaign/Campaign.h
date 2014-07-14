#pragma once

#include "engine/common/Pointers.h"
#include <vector>
#include <string>
#include <map>
#include <inttypes.h>
#include "CampaignMap.h"

class IGameStatePersister;

class Campaign {
public:
	typedef std::vector<CampaignMapPtr> MapList;
	typedef MapList::iterator MapListIter;
	typedef MapList::const_iterator MapListConstIter;

private:
	MapList _maps;
	std::string _id;
	typedef std::map<std::string, std::string> SettingsMap;
	typedef SettingsMap::const_iterator SettingsMapConstIter;
	SettingsMap _settings;
	int _currentMap;
	IGameStatePersister* _persister;
	uint8_t _lives;

public:
	Campaign (const std::string& id, IGameStatePersister* persister);

	virtual ~Campaign ();

	CampaignMapPtr getMap (int index) const;

	CampaignMap* getMapById (const std::string& mapname);

	void resetMaps ();

	bool loadProgress ();

	bool saveProgress ();

	bool reset (bool unlockFirstMap = true);

	void setLives (uint8_t lives);

	uint8_t getLives () const;

	const MapList& getMaps () const;

	bool unlock ();

	const std::string& getId () const;

	std::string getIcon () const;

	std::string getText () const;

	void setSetting (const std::string& key, const std::string& value);

	bool hasMoreMaps () const;

	void addMap (const std::string& id, const std::string& name);

	// returns false is no more maps are available to unlock
	bool unlockNextMap (bool shouldSaveProgress = true);

	std::string getNextMap () const;

	// will return a setting for a campaign from the map definition.
	std::string getSetting (const std::string& key) const;

	std::string toString() const;

	bool isUnlocked () const;
};

inline const Campaign::MapList& Campaign::getMaps () const
{
	return _maps;
}

inline const std::string& Campaign::getId () const
{
	return _id;
}

inline std::string Campaign::getIcon () const
{
	return getSetting("icon");
}

inline std::string Campaign::getText () const
{
	return getSetting("text");
}

typedef SharedPtr<Campaign> CampaignPtr;

class ICampaignVisitor {
public:
	virtual ~ICampaignVisitor ()
	{
	}

	virtual void visitCampaign (CampaignPtr& campaign) = 0;
};
