#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodePoint.h"
#include "engine/client/ui/nodes/UINodeMapOnScreenCursorControl.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
	IUIMapWindow(frontend, serviceProvider, campaignManager, map, new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0, frontend->getWidth(), frontend->getHeight(), map))
{
	init();

	_musicFile = "";
}

void UIMapWindow::showAutoSolveSlider()
{
}

void UIMapWindow::initHudNodes ()
{
	UINode* panel = new UINode(_frontend);
	panel->setImage("bones");
	panel->setStandardPadding();
	panel->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_CENTER);
	add(panel);

	UINodePoint* points = new UINodePoint(_frontend, 30);
	points->setLabel("00000");
	points->setPos(panel->getX() + 0.02f, panel->getY());
	points->setId(UINODE_POINTS);
	add(points);
}

UINode* UIMapWindow::getFingerControl ()
{
	UINodeMapOnScreenCursorControl* node = new UINodeMapOnScreenCursorControl(_frontend, _nodeMap);
	_mapControl = node;

	UINode* undo = new UINode(_frontend);
	undo->setImage("icon-undo");
	undo->setStandardPadding();
	undo->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_RIGHT);
	undo->setOnActivate("undo");
	add(undo);

	return node;
}
