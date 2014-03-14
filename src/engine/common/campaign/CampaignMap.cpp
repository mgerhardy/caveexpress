#include "CampaignMap.h"
#include <string>
#include <sstream>

CampaignMap::CampaignMap (const std::string& id, const std::string& name, bool locked) :
		_id(id), _name(name), _initialLockedState(locked)
{
	reset();
}

CampaignMap::~CampaignMap ()
{
}

void CampaignMap::initialUnlock ()
{
	_initialLockedState = _locked = false;
}

void CampaignMap::setStars (uint8_t stars)
{
	_stars = stars;
}

void CampaignMap::setFinishPoints (uint32_t finishPoints)
{
	_finishPoints = finishPoints;
}

void CampaignMap::setTime (uint32_t time)
{
	// only use the best time
	if (_time == 0 || time < _time)
		_time = time;
}

void CampaignMap::reset ()
{
	_time = 0;
	_finishPoints = 0;
	_stars = 0;
	_locked = _initialLockedState;
}

std::string CampaignMap::toString() const
{
	std::stringstream s;
	s << "CampaignMap[";
	s << "id: " << _id << ", ";
	s << "name: " << _name << ", ";
	s << "time: " << _time << ", ";
	s << "finishPoints: " << _finishPoints << ", ";
	s << "locked: " << _locked << ", ";
	s << "initialLockedState: " << _initialLockedState << ", ";
	s << "stars: " << (int)_stars;
	s << "]";
	return s.str();
}
