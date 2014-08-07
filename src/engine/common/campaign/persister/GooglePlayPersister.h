#pragma once

#include "IGameStatePersister.h"

class GooglePlayPersister: public IGameStatePersister {
public:
	GooglePlayPersister();
	virtual ~GooglePlayPersister();

	bool saveCampaign (Campaign*) override;
	bool loadCampaign (Campaign*) override;
	bool reset () override;
	bool resetCampaign (Campaign*) override;
};
