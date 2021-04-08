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
	shutdown();
}

void ServiceProvider::updateNetwork (bool network)
{
	INetwork* newNetwork;
	if (network) {
		newNetwork = _network;
	} else {
		newNetwork = _loopback;
	}

	// no need to switch
	if (newNetwork == _currentNetwork)
		return;

	if (network) {
		Log::info(LOG_SERVICE, "switching to network");
	} else {
		Log::info(LOG_SERVICE, "switching to loopback");
	}

	if (_currentNetwork) {
		_currentNetwork->shutdown();
	}
	_currentNetwork = newNetwork;
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

void ServiceProvider::shutdown()
{
	if (_currentNetwork)
		_currentNetwork->shutdown();

	delete _mapManager;
#ifndef NONETWORK
	delete _network;
#endif
	delete _loopback;
	delete _textureDefinition;

	_mapManager = nullptr;
	_network = nullptr;
	_loopback = nullptr;
	_textureDefinition = nullptr;

	_currentNetwork = nullptr;
}

void ServiceProvider::init (IFrontend *frontend, EventHandler *eventHandler)
{
	_eventHandler = eventHandler;
	{
		const ExecutionTime e("loading network");
		_loopback = new NoNetwork();
#ifndef NONETWORK
		if (Config.isNetwork())
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
