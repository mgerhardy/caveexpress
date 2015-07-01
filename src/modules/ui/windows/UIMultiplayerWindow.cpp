#include "UIMultiplayerWindow.h"
#include "ui/nodes/UINodeBackground.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/nodes/UINodeMainButton.h"
#include "ui/nodes/UINodeTextInput.h"
#include "ui/windows/listener/ConfigVarListener.h"
#include "ui/nodes/UINodeServerSelector.h"
#include "common/MapManager.h"
#include "common/CommandSystem.h"
#include "ui/UI.h"
#include "client/commands/CmdServerPing.h"
#include "ui/windows/listener/OpenWindowListener.h"
#include "service/ServiceProvider.h"
#include "common/Commands.h"

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
