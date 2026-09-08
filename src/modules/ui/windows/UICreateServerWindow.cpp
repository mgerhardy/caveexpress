#include "UICreateServerWindow.h"
#include "ui/nodes/UINodeMapSelector.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSpinner.h"
#include "ui/windows/UIWindow.h"
#include "ui/windows/listener/ConfigVarListener.h"
#include "ui/UI.h"
#include "common/ConfigManager.h"
#include "common/LobbyPlayers.h"
#include "network/INetwork.h"
#include <algorithm>

UICreateServerWindow::UICreateServerWindow (IFrontend *frontend, const IMapManager &mapManager) :
		UIMapSelectorWindow(new UINodeMapSelector(this, frontend, mapManager, true), tr("Create server"), UI_WINDOW_CREATE_SERVER, frontend),
		_maxPlayers(nullptr)
{
	UINodeLabel *label = new UINodeLabel(frontend, tr("Max players"));
	label->setFont(LARGE_FONT);
	label->setColor(colorWhite);
	label->putAbove(_mapSelector);
	add(label);

	_maxPlayers = new UINodeSpinner(frontend, lobby::MIN_SESSION_PLAYERS, MAX_CLIENTS, 1);
	_maxPlayers->setId("maxplayers");
	_maxPlayers->setBackgroundColor(colorWhite);
	_maxPlayers->addListener(UINodeListenerPtr(new ConfigVarListener("maxplayers", _maxPlayers)));
	const int value = lobby::clampSessionMaxPlayers(
			Config.getConfigVar("maxplayers", "2")->getIntValue(), MAX_CLIENTS);
	_maxPlayers->setValue(value);
	_maxPlayers->setSize(std::max(_maxPlayers->getAutoWidth(), 0.08f), std::max(label->getHeight(), 0.04f));
	_maxPlayers->putRight(label, 0.02f);
	add(_maxPlayers);
}

UICreateServerWindow::~UICreateServerWindow ()
{
}

void UICreateServerWindow::onActive ()
{
	UIMapSelectorWindow::onActive();
	const int value = lobby::clampSessionMaxPlayers(
			Config.getConfigVar("maxplayers", "2")->getIntValue(), MAX_CLIENTS);
	Config.getConfigVar("maxplayers")->setValue(value);
	if (_maxPlayers != nullptr)
		_maxPlayers->setValue(value);
}
