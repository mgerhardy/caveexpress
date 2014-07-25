#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeMapOnScreenCursorControl.h"

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
	IUIMapWindow(frontend, serviceProvider, campaignManager, map, new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0, frontend->getWidth(), frontend->getHeight(), map))
{
	init();
	_timeBar->setVisible(false);
	_hitpointsBar->setVisible(false);
	_livesSprite->setVisible(false);
}

UINode* UIMapWindow::getFingerControl ()
{
	UINodeMapOnScreenCursorControl* node = new UINodeMapOnScreenCursorControl(_frontend, _nodeMap);
	_mapControl = node ;
	return node;
}
