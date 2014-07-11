#include "CavePacker.h"
#include "engine/client/ui/UI.h"
#include "cavepacker/client/ui/windows/UIMainWindow.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"
#include "cavepacker/client/CavePackerClientMap.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/ExecutionTime.h"

CavePacker::CavePacker ():
	_persister(nullptr), _campaignManager(nullptr), _map(nullptr), _frontend(nullptr), _serviceProvider(nullptr)
{
	delete _persister;
	delete _campaignManager;
	delete _map;
}

CavePacker::~CavePacker ()
{
}

void CavePacker::initSoundCache ()
{
}

void CavePacker::connect ()
{
}

void CavePacker::update (uint32_t deltaTime)
{
}

void CavePacker::onData (ClientId clientId, ByteStream &data)
{
}

bool CavePacker::mapLoad (const std::string& map)
{
	return false;
}

void CavePacker::mapReload ()
{
}

void CavePacker::mapShutdown ()
{
}

void CavePacker::connect (ClientId clientId)
{
}

UIWindow* CavePacker::createPopupWindow (IFrontend* frontend, const std::string& text, int flags, UIPopupCallbackPtr callback)
{
	return nullptr;
}

int CavePacker::disconnect (ClientId clientId)
{
	return 0;
}

int CavePacker::getPlayers ()
{
	return 0;
}

std::string CavePacker::getMapName ()
{
	return "";
}

void CavePacker::shutdown ()
{
}

void CavePacker::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;

	{
		ExecutionTime e("loading persister");
		_persister = new NOPPersister();
	}
	{
		ExecutionTime e("campaign manager");
		_campaignManager = new CampaignManager(_persister, serviceProvider.getMapManager());
		_campaignManager->init();
	}
}

void CavePacker::initUI (IFrontend* frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();
	ui.addWindow(new UIMainWindow(frontend));
	CavePackerClientMap *map = new CavePackerClientMap(0, 0, _frontend->getWidth(), _frontend->getHeight(), _frontend, serviceProvider, UI::get().loadTexture("tile-reference")->getWidth());
	_map = map;
	ui.addWindow(new UIMapWindow(frontend, serviceProvider, *_campaignManager, *_map));
}
