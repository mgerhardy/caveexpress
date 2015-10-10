#include "ServiceProvider.h"
#include "common/MapManager.h"
#include "common/ConfigManager.h"
#ifndef NONETWORK
#include "network/Network.h"
#endif
#include "network/NoNetwork.h"
#include "common/TextureDefinition.h"
#include "common/IFrontend.h"
#include "game/GameRegistry.h"
#include "common/ExecutionTime.h"
#include "common/Log.h"

ServiceProvider::ServiceProvider() :
		_mapManager(nullptr), _network(nullptr), _loopback(nullptr), _textureDefinition(nullptr), _currentNetwork(nullptr)
{
}

ServiceProvider::~ServiceProvider ()
{
	Log::info(LOG_SERVICE, "shutting down the serviceprovider");
	if (_mapManager != nullptr)
		delete _mapManager;
	if (_network != nullptr)
		delete _network;
	if (_loopback != nullptr)
		delete _loopback;
	if (_textureDefinition != nullptr)
		delete _textureDefinition;
}

void ServiceProvider::updateNetwork (bool network)
{
	if (_currentNetwork)
		_currentNetwork->shutdown();

	if (network) {
		_currentNetwork = _network;
		Log::info(LOG_SERVICE, "switching to network");
	} else {
		_currentNetwork = _loopback;
		Log::info(LOG_SERVICE, "switching to loopback");
	}
	_currentNetwork->init();
}

void ServiceProvider::initTextureDefinition (IFrontend *frontend, const std::string& textureSize, IProgressCallback* progress)
{
	Log::info(LOG_SERVICE, "initialize the texture definition");
	if (_textureDefinition != nullptr)
		delete _textureDefinition;
	ExecutionTime e("texture definition");
	if (textureSize == "auto") {
		if (System.isSmallScreen(frontend)) {
			_textureDefinition = new TextureDefinition("small", progress);
		} else {
			_textureDefinition = new TextureDefinition("big", progress);
		}
	} else {
		_textureDefinition = new TextureDefinition(textureSize, progress);
	}
}

void ServiceProvider::init (IFrontend *frontend)
{
	{
		const ExecutionTime e("loading network");
		_loopback = new NoNetwork();
#ifndef NONETWORK
		if (Config.get().isNetwork())
			_network = new Network();
		else
#endif
			_network = _loopback;
		updateNetwork(false);
	}
	initTextureDefinition(frontend, Config.getTextureSize());
	{
		const ExecutionTime e("map manager");
		_mapManager = Singleton<GameRegistry>::getInstance().getGame()->getMapManager();
		if (_mapManager != nullptr)
			_mapManager->init();
	}
	Log::info(LOG_SERVICE, "initialized the serviceprovider");
}
