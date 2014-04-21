#include "ServiceProvider.h"
#include "engine/common/MapManager.h"
#include "engine/common/ConfigManager.h"
#ifndef NONETWORK
#include "engine/common/network/Network.h"
#endif
#include "engine/common/network/NoNetwork.h"
#include "engine/common/TextureDefinition.h"
#include "engine/common/IFrontend.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/Logger.h"

ServiceProvider::ServiceProvider() :
		_mapManager(nullptr), _network(nullptr), _loopback(nullptr), _textureDefinition(nullptr), _currentNetwork(nullptr)
{
}

ServiceProvider::~ServiceProvider ()
{
	info(LOG_GENERAL, "shutting down the serviceprovider");
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
		info(LOG_GENERAL, "switching to network");
	} else {
		_currentNetwork = _loopback;
		info(LOG_GENERAL, "switching to loopback");
	}
	_currentNetwork->init();
}

void ServiceProvider::initTextureDefinition (IFrontend *frontend, const std::string& textureSize, IProgressCallback* progress)
{
	if (_textureDefinition != nullptr)
		delete _textureDefinition;
	ExecutionTime e("texture definition");
	if (textureSize == "auto") {
		if (System.isSmallScreen() || frontend->getHeight() <= 768 || frontend->getWidth() <= 1024) {
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
		ExecutionTime e("loading network");
#ifndef NONETWORK
		if (Config.get().isNetwork())
			_network = new Network();
		else
#endif
			_network = new NoNetwork();
		_loopback = new NoNetwork();
		updateNetwork(false);
	}
	initTextureDefinition(frontend, Config.getTextureSize());
	{
		ExecutionTime e("map manager");
		_mapManager = new MapManager();
		_mapManager->init();
	}
}
