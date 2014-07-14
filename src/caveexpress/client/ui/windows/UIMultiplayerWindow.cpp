#include "UIMultiplayerWindow.h"
#include "caveexpress/client/ui/nodes/UINodeBackground.h"
#include "engine/client/ui/nodes/UINodeBackButton.h"
#include "engine/client/ui/nodes/UINodeMainButton.h"
#include "engine/client/ui/nodes/UINodeTextInput.h"
#include "engine/client/ui/windows/listener/ConfigVarListener.h"
#include "engine/client/ui/nodes/UINodeServerSelector.h"
#include "engine/common/MapManager.h"
#include "engine/client/ui/UI.h"
#include "engine/client/commands/CmdServerPing.h"
#include "engine/client/ui/windows/listener/OpenWindowListener.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/Commands.h"
#include "caveexpress/shared/constants/Commands.h"

UIMultiplayerWindow::UIMultiplayerWindow (IFrontend *frontend, const IMapManager &mapManager, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_MULTIPLAYER, frontend), _serviceProvider(serviceProvider)
{
	UINodeBackground *background = new UINodeBackground(frontend, tr("Multiplayer"));
	add(background);

	UINodeServerSelector *serverlist = new UINodeServerSelector(frontend);
	serverlist->setId(UINODE_SERVERSELECTOR);
	add(serverlist);

	UINodeMainButton *refresh = new UINodeMainButton(frontend, tr("Refresh"));
	refresh->putUnderRight(serverlist);
	refresh->setOnActivate(CMD_CL_PINGSERVERS);
	add(refresh);

	UINodeMainButton *server = new UINodeMainButton(frontend, tr("Open"));
	server->putLeft(refresh);
	server->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_CREATE_SERVER)));
	add(server);

	UINodeTextInput *userName = new UINodeTextInput(_frontend, LARGE_FONT);
	userName->setBackgroundColor(colorWhite);
	userName->setValue(Config.getName());
	userName->addListener(UINodeListenerPtr(new ConfigVarListener("name", userName)));
	userName->putLeft(server);
	add(userName);
	Commands.registerCommand(CMD_CL_PINGSERVERS, new CmdServerPing(serverlist, serviceProvider));

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UIMultiplayerWindow::~UIMultiplayerWindow ()
{
	Commands.removeCommand(CMD_CL_PINGSERVERS);
}

void UIMultiplayerWindow::onActive ()
{
	_serviceProvider.updateNetwork(true);
	Commands.executeCommand(CMD_CL_PINGSERVERS);
	UIWindow::onActive();
}

bool UIMultiplayerWindow::onPop ()
{
	const bool retVal = UIWindow::onPop();
	if (retVal)
		_serviceProvider.updateNetwork(false);
	return retVal;
}
