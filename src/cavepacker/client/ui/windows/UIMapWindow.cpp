#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodePoint.h"
#include "engine/client/ui/nodes/UINodeMapOnScreenCursorControl.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "engine/client/ui/UI.h"
#include "engine/common/ConfigManager.h"

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
		IUIMapWindow(frontend, serviceProvider, campaignManager, map,
				new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0,
						frontend->getWidth(), frontend->getHeight(), map)), _undo(nullptr) {
	init();

	_autoSolveSlider = new UINodeSlider(frontend, 100.0f, 1000.0f, 100.0f);
	_autoSolveSlider->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	add(_autoSolveSlider);
	hideAutoSolveSlider();

	if (campaignManager.firstMap())
		UI::get().push("introgame");
}

void UIMapWindow::update (uint32_t deltaTime)
{
	IUIMapWindow::update(deltaTime);
	if (_autoSolveSlider->isVisible()) {
		Config.getConfigVar("solvestepmillis")->setValue(string::toString(_autoSolveSlider->getValue()));
	}
}

void UIMapWindow::showAutoSolveSlider()
{
	_autoSolveSlider->setVisible(true);
}

void UIMapWindow::hideAutoSolveSlider()
{
	_autoSolveSlider->setVisible(false);
}

void UIMapWindow::hideHud()
{
	IUIMapWindow::hideHud();
	_undo->setVisible(false);
}

void UIMapWindow::showHud()
{
	IUIMapWindow::showHud();
	_undo->setVisible(true);
	hideAutoSolveSlider();
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

	_undo = new UINode(_frontend);
	_undo->setImage("icon-undo");
	_undo->setStandardPadding();
	_undo->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_RIGHT);
	_undo->setOnActivate("undo");
	add(_undo);

	return node;
}
