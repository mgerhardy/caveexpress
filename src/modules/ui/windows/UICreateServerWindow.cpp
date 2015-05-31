#include "UICreateServerWindow.h"
#include "ui/nodes/UINodeMapSelector.h"
#include "ui/windows/UIWindow.h"
#include "ui/UI.h"

UICreateServerWindow::UICreateServerWindow (IFrontend *frontend, const IMapManager &mapManager) :
		UIMapSelectorWindow(new UINodeMapSelector(frontend, mapManager, true), tr("Create server"), UI_WINDOW_CREATE_SERVER, frontend)
{
	// TODO: multiplayer options like a password and a max client setting
}

UICreateServerWindow::~UICreateServerWindow ()
{
}
