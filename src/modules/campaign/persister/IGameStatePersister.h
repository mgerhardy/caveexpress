#pragma once

#include "common/Compiler.h"
#include "common/Log.h"
#include "campaign/Campaign.h"
#include <string>

#define DEFAULT_CAMPAIGN "tutorial"

namespace {
const int INITIAL_LIVES = 3;
}

class IGameStatePersister {
protected:
	std::string _activeCampaign;

public:
	explicit IGameStatePersister (const std::string& initialCampaign) :
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
		Log::info(LOG_CAMPAIGN, "don't persist campaign progress for %s", campaign->getId().c_str());
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
