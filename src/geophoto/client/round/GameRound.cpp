#include "GameRound.h"
#include "geophoto/client/round/JSONGameRoundParser.h"
#include "engine/common/Logger.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/FileSystem.h"
#include "engine/common/System.h"

#define ROUNDLENGTH 10

GameRound::GameRound (IGameStatePersister& persister, IProgressCallback *callback) :
		_persister(persister), _callback(callback)
{
}

GameRound::~GameRound ()
{
}

void GameRound::init ()
{
	if (_callback != nullptr)
		_callback->progressInit(ROUNDLENGTH + 1, "Generating round");
	const URI apiUrl(string::replaceAll(Config.getUrl(), "$difficulty$", Config.getDifficulty()));
	FilePtr file = FS.get().getFile(apiUrl);
	if (_callback != nullptr)
		_callback->progressStep("Loading images");
	const JSONGameRoundParser p(file, _callback);
	if (_callback != nullptr)
		_callback->progressDone();
	if (!p.wasSuccess()) {
		System.exit("Failed to parse the json data from " + file->getURI().print(), 1);
	}
	_allLocations = _locations = p.getLocations();
	info(LOG_CLIENT, "fetched " + string::toString(_locations.size()) + " locations");
}

bool GameRound::hasMoreLocations () const
{
	return !_locations.empty();
}

int GameRound::activateNextLocation ()
{
	if (!hasMoreLocations())
		return -1;

	_currentLocation = _locations.back();
	_locations.pop_back();
	return _currentLocation.id;
}

void GameRound::save ()
{
	_persister.saveRound(*this);
}
