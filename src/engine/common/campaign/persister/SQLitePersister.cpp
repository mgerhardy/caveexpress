#include "SQLitePersister.h"
#include "../Campaign.h"
#include "GameStateSQLite.h"
#include "engine/common/FileSystem.h"

SQLitePersister::SQLitePersister (const std::string& filename) :
		IGameStatePersister(), _filename(filename), _gameState(new GameStateSQLite(filename))
{
	_activeCampaign = _gameState->getActiveCampaign();
}

SQLitePersister::~SQLitePersister ()
{
	delete _gameState;
}

bool SQLitePersister::saveCampaign (Campaign* campaign)
{
	_activeCampaign = campaign->getId();
	return _gameState->updateCampaign(campaign);
}

bool SQLitePersister::reset ()
{
	_activeCampaign = DEFAULT_CAMPAIGN;
	delete _gameState;
	_gameState = new GameStateSQLite(_filename);
	return true;
}

bool SQLitePersister::loadCampaign (Campaign* campaign)
{
	_activeCampaign = campaign->getId();
	return _gameState->loadCampaign(campaign);
}

bool SQLitePersister::resetCampaign (Campaign* campaign)
{
	_activeCampaign = campaign->getId();
	return _gameState->resetState(campaign);
}
