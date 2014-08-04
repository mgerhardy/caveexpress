#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"
#include "cavepacker/client/ui/nodes/UICavePackerNodePoint.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodePoint.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeMapOnScreenCursorControl.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "engine/client/ui/UI.h"
#include "engine/common/ConfigManager.h"

class SolveListener: public UINodeListener {
private:
	UINodeSlider *_sliderNode;
	std::string _configVar;
public:
	SolveListener (UINodeSlider *sliderNode, const std::string& configVar) :
			_sliderNode(sliderNode), _configVar(configVar)
	{
		_sliderNode->setValue(Config.getConfigVar(_configVar)->getFloatValue());
	}

	void onValueChanged () override
	{
		const float val = _sliderNode->getValue();
		Config.getConfigVar(_configVar)->setValue(string::toString(val));
	}
};

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
		IUIMapWindow(frontend, serviceProvider, campaignManager, map,
				new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0,
						frontend->getWidth(), frontend->getHeight(), map)), _undo(
				nullptr), _points(nullptr), _campaignManager(campaignManager) {
	init();

	const float height = 0.05f;
	const float sliderWidth = 0.3f;
	_autoSolveSlider = new UINodeSlider(frontend, 100.0f, 1000.0f, 100.0f);
	_autoSolveSlider->setAlignment(NODE_ALIGN_BOTTOM | NODE_ALIGN_CENTER);
	_autoSolveSlider->setSize(sliderWidth, height);
	_autoSolveSlider->addListener(UINodeListenerPtr(new SolveListener(_autoSolveSlider, "solvestepmillis")));
	add(_autoSolveSlider);
	hideAutoSolveSlider();
}

void UIMapWindow::showAutoSolveSlider()
{
	_autoSolveSlider->setVisible(true);
}

void UIMapWindow::hideAutoSolveSlider()
{
	_autoSolveSlider->setVisible(false);
}

void UIMapWindow::showCursor (bool /*show*/)
{
	IUIMapWindow::showCursor(true);
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

	UINode *innerPanel = new UINode(_frontend);
	innerPanel->setLayout(new UIHBoxLayout(0.01f));
	innerPanel->setPos(panel->getX() + 0.07f, panel->getY());
	_points = new UICavePackerNodePoint(_frontend, 30);
	_points->setFont(HUGE_FONT);
	_points->setId(UINODE_POINTS);
	innerPanel->add(_points);
	add(innerPanel);
}

void UIMapWindow::initInputHudNodes ()
{
	IUIMapWindow::initInputHudNodes();
	_undo = new UINodeButton(_frontend);
	_undo->setImage("icon-undo");
	_undo->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_RIGHT);
	_undo->setOnActivate("undo");
	add(_undo);
}

UINode* UIMapWindow::getFingerControl ()
{
	UINodeMapOnScreenCursorControl* node = new UINodeMapOnScreenCursorControl(_frontend, _nodeMap);
	_mapControl = node;
	return node;
}

void UIMapWindow::initWaitingForPlayers (bool adminOptions) {
	IUIMapWindow::initWaitingForPlayers(adminOptions);
	ClientMap& map = _nodeMap->getMap();
	const std::string& name = map.getName();
	const int ownBest = _campaignManager.getActiveCampaign()->getMapById(name)->getFinishPoints();
	const std::string best = map.getSetting("best", "0" /* string::toString(ownBest) */);
	info(LOG_CLIENT, "got best points from server: " + best);
	_points->setOwnAndGlobalBest(ownBest, string::toInt(best));
	_points->setLabel("0");

	if (_campaignManager.firstMap())
		UI::get().push("introgame");
}
