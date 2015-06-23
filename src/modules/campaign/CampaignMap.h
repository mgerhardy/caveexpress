#pragma once

#include "common/Pointers.h"
#include "common/Log.h"
#include <string>
#include <inttypes.h>

class CampaignMap {
private:
	std::string _id;
	std::string _name;
	bool _locked;
	bool _initialLockedState;
	uint32_t _time;
	uint32_t _finishPoints;
	uint8_t _stars;

public:
	CampaignMap (const std::string& id, const std::string& name, bool locked);

	virtual ~CampaignMap ();

	void reset ();
	void initialUnlock ();

	void lock ();
	void unlock ();

	bool isLocked () const;

	const std::string& getId () const;
	const std::string& getName () const;

	void setStars (uint8_t stars);
	uint8_t getStars () const;

	void setFinishPoints (uint32_t finishPoints);
	uint32_t getFinishPoints () const;

	void setTime (uint32_t time);
	uint32_t getTime () const;

	std::string toString() const;
};

typedef SharedPtr<CampaignMap> CampaignMapPtr;

inline void CampaignMap::unlock ()
{
	Log::trace(LOG_CAMPAIGN, "unlock map %s", _id.c_str());
	_locked = false;
}

inline void CampaignMap::lock ()
{
	Log::trace(LOG_CAMPAIGN, "lock map %s", _id.c_str());
	_locked = true;
}

inline bool CampaignMap::isLocked () const
{
	return _locked;
}

inline const std::string& CampaignMap::getId () const
{
	return _id;
}

inline const std::string& CampaignMap::getName () const
{
	return _name;
}

inline uint32_t CampaignMap::getTime () const
{
	return _time;
}

inline uint32_t CampaignMap::getFinishPoints () const
{
	return _finishPoints;
}

inline uint8_t CampaignMap::getStars () const
{
	return _stars;
}
