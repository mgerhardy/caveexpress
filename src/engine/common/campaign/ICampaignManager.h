#pragma once

#include <string>
#include <stdint.h>
#include "Campaign.h"

class ICampaignManager {
public:
	virtual ~ICampaignManager () {}

	virtual void reset () = 0;

	virtual CampaignPtr getActiveCampaign () const = 0;

	virtual bool updateMapValues (const std::string& mapname, uint32_t finishPoints, uint32_t time, uint8_t stars, bool lowerPointsAreBetter = false) = 0;
};
