#include "ServiceProvider.h"
#include "client/round/IGameStatePersister.h"
#include "client/round/persister/GameStateSQLite.h"
#include "shared/ConfigManager.h"
#include "shared/network/Network.h"
#include "shared/network/NoNetwork.h"
#include "shared/TextureDefinition.h"
#include "shared/IFrontend.h"

ServiceProvider::ServiceProvider () :
		_persister(nullptr), _network(nullptr), _textureDefinition(nullptr)
{
}

ServiceProvider::~ServiceProvider ()
{
	if (_persister != nullptr)
		delete _persister;
	if (_network != nullptr)
		delete _network;
	if (_textureDefinition != nullptr)
		delete _textureDefinition;
}

void ServiceProvider::init (IFrontend *frontend)
{
	const ConfigVarPtr& persister = Config.get().getConfigVar("persister", "sqlite", true, CV_READONLY);
	if (persister->getValue() == "nop")
		_persister = new NOPPersister();
	else
		_persister = new GameStateSQLite();

	const ConfigVarPtr& network = Config.get().getConfigVar("network", "true", true);
	if (network->getBoolValue())
		_network = new Network();
	else
		_network = new NoNetwork();

	std::string textureSize = ConfigManager::get().getTextureSize();
	if (textureSize == "auto") {
		if (frontend->getHeight() > 768 || frontend->getWidth() > 1024)
			textureSize = "big";
		else
			textureSize = "small";
	}
	_textureDefinition = new TextureDefinition(textureSize);
}
