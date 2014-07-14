#include "UICreateServerWindow.h"
#include "caveexpress/client/ui/nodes/UINodeMapSelector.h"
#include "engine/client/ui/windows/UIWindow.h"
#include "engine/client/ui/UI.h"

UICreateServerWindow::UICreateServerWindow (IFrontend *frontend, const IMapManager &mapManager) :
		UIMapSelectorWindow(new UINodeMapSelector(frontend, mapManager), tr("Create server"), UI_WINDOW_CREATE_SERVER, frontend)
{
	// TODO: multiplayer options like a password and a max client setting
}

UICreateServerWindow::~UICreateServerWindow ()
{
}
