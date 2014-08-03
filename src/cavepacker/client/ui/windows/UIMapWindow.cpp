#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodePoint.h"
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
						frontend->getWidth(), frontend->getHeight(), map)), _undo(nullptr) {
	init();

	const float height = 0.05f;
	const float sliderWidth = 0.3f;
	_autoSolveSlider = new UINodeSlider(frontend, 100.0f, 1000.0f, 100.0f);
	_autoSolveSlider->setAlignment(NODE_ALIGN_BOTTOM | NODE_ALIGN_CENTER);
	_autoSolveSlider->setSize(sliderWidth, height);
	_autoSolveSlider->addListener(UINodeListenerPtr(new SolveListener(_autoSolveSlider, "solvestepmillis")));
	add(_autoSolveSlider);
	hideAutoSolveSlider();

	if (campaignManager.firstMap())
		UI::get().push("introgame");
}

void UIMapWindow::showAutoSolveSlider()
{
	UI::get().showCursor(true);
	_autoSolveSlider->setVisible(true);
}

void UIMapWindow::hideAutoSolveSlider()
{
	UI::get().showCursor(false);
	_autoSolveSlider->setVisible(false);
}

void UIMapWindow::hideHud()
{
	IUIMapWindow::hideHud();
	if (_undo)
		_undo->setVisible(false);
}

void UIMapWindow::showHud()
{
	IUIMapWindow::showHud();
	if (_undo)
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
	innerPanel->setPos(panel->getX() + 0.03f, panel->getY());
	UINodeLabel* label = new UINodeLabel(_frontend, tr("Steps"), getFont(HUGE_FONT));
	label->setColor(colorWhite);
	innerPanel->add(label);
	UINodePoint* points = new UINodePoint(_frontend, 30);
	points->setId(UINODE_POINTS);
	innerPanel->add(points);
	add(innerPanel);
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
