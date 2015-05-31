#pragma once

#include "engine/common/Compiler.h"
#include "engine/common/Logger.h"
#include "engine/common/campaign/Campaign.h"
#include <string>
#include <stdint.h>
#include <map>

#define DEFAULT_CAMPAIGN "tutorial"

namespace {
const int INITIAL_LIVES = 3;
}

class IGameStatePersister {
protected:
	std::string _activeCampaign;

public:
	IGameStatePersister (const std::string& initialCampaign) :
			_activeCampaign(initialCampaign)
	{
	}

	IGameStatePersister () :
			_activeCampaign(DEFAULT_CAMPAIGN)
	{
	}

	virtual ~IGameStatePersister ()
	{
	}

	virtual bool saveCampaign (Campaign*) = 0;
	virtual bool loadCampaign (Campaign*) = 0;
	virtual bool reset () = 0;
	virtual bool resetCampaign (Campaign*) = 0;

	virtual bool init () = 0;

	virtual const std::string& getActiveCampaign () const
	{
		return _activeCampaign;
	}
};

class NOPPersister: public IGameStatePersister {
public:
	NOPPersister () :
			IGameStatePersister()
	{
	}

	virtual bool init() override
	{
		return true;
	}

	virtual bool saveCampaign (Campaign* campaign) override
	{
		info(LOG_CAMPAIGN, "don't persist campaign progress for " + campaign->getId());
		_activeCampaign = campaign->getId();
		return true;
	}

	virtual bool loadCampaign (Campaign* campaign) override
	{
		campaign->setLives(INITIAL_LIVES);
		return true;
	}

	virtual bool resetCampaign (Campaign* campaign) override
	{
		campaign->setLives(INITIAL_LIVES);
		return true;
	}

	virtual bool reset () override
	{
		return true;
	}
};
