#include "CavePacker.h"
#include "engine/client/ui/UI.h"
#include "cavepacker/client/ui/windows/UIMainWindow.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"

CavePacker::CavePacker ()
{
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
}

void CavePacker::initUI (IFrontend* _frontend, ServiceProvider& serviceProvider)
{
	UI& ui = UI::get();
	ui.addWindow(new UIMainWindow(_frontend));
	ui.addWindow(new UIMapWindow(_frontend));
}
