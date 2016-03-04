#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"
#include "cavepacker/client/ui/nodes/UICavePackerNodePoint.h"
#include "cavepacker/shared/network/messages/MoveToMessage.h"
#include "listener/SolveListener.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/UI.h"
#include "client/Camera.h"
#include "common/ConfigManager.h"
#include "common/Commands.h"
#include "service/ServiceProvider.h"
#include "common/Log.h"
#include "common/IFrontend.h"

namespace cavepacker {

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, CavePackerClientMap& map) :
		IUIMapWindow(frontend, serviceProvider, campaignManager,
				new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0,
						frontend->getWidth(), frontend->getHeight(), map), false), _undo(
				nullptr), _points(nullptr), _campaignManager(campaignManager), _scrolling(false), _targetX(-1), _targetY(-1) {
	init();

	const float height = 0.05f;
	const float sliderWidth = 0.3f;
	_autoSolveSlider = new UINodeSlider(frontend, 10.0f, 1000.0f, 10.0f);
	_autoSolveSlider->setAlignment(NODE_ALIGN_BOTTOM | NODE_ALIGN_CENTER);
	_autoSolveSlider->setSize(sliderWidth, height);
	_autoSolveSlider->addListener(UINodeListenerPtr(new SolveListener(_autoSolveSlider, "solvestepmillis")));
	_autoSolveSlider->setId("autosolveslider");
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
	panel->setId("hudpanel_bonus");
	panel->setImage("bones");
	panel->setStandardPadding();
	panel->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_CENTER);
	add(panel);

	UINode *innerPanel = new UINode(_frontend);
	innerPanel->setId("hudpanel_inner");
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
	_undo->setId("undo");
	_undo->setImage("icon-undo");
	_undo->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_RIGHT);
	_undo->setOnActivate("undo");
	add(_undo);

	if (Config.getConfigVar("forcefingercontrol", "false", true, CV_READONLY)->getBoolValue() || System.hasTouch()) {
		UINodeButton* left = new UINodeButton(_frontend);
		left->setImage("icon-cursor-left");
		left->setOnActivate(CMD_MOVE_LEFT);
		left->setTriggerTime(500u);
		left->setAlignment(NODE_ALIGN_LEFT | NODE_ALIGN_BOTTOM);
		add(left);
		UINodeButton* right = new UINodeButton(_frontend);
		right->setImage("icon-cursor-right");
		right->setOnActivate(CMD_MOVE_RIGHT);
		right->setTriggerTime(500u);
		right->putRight(left);
		add(right);

		UINodeButton* down = new UINodeButton(_frontend);
		down->setImage("icon-cursor-down");
		down->setOnActivate(CMD_MOVE_DOWN);
		down->setAlignment(NODE_ALIGN_RIGHT | NODE_ALIGN_BOTTOM);
		down->setTriggerTime(500u);
		add(down);
		UINodeButton* up = new UINodeButton(_frontend);
		up->setImage("icon-cursor-up");
		up->setOnActivate(CMD_MOVE_UP);
		up->setTriggerTime(500u);
		up->putAbove(down);
		add(up);
	}
}

UINode* UIMapWindow::getFingerControl ()
{
	return nullptr;
}

void UIMapWindow::initWaitingForPlayers (bool adminOptions) {
	IUIMapWindow::initWaitingForPlayers(adminOptions);
	ClientMap& map = _nodeMap->getMap();
	const std::string& name = map.getName();
	const CampaignPtr& c = _campaignManager.getActiveCampaign();
	const CampaignMap* campaignMap = c->getMapById(name);
	const int ownBest = campaignMap != nullptr ? campaignMap->getFinishPoints() : 0;
	const std::string best = map.getSetting("best", "0" /* string::toString(ownBest) */);
	Log::info(LOG_GAMEIMPL, "got best points from server: %s", best.c_str());
	_points->setOwnAndGlobalBest(ownBest, string::toInt(best));
	_points->setLabel("0");

	if (_serviceProvider.getNetwork().isMultiplayer())
		return;

	if (name == "tutorial0001") {
		UI::get().push("introgame");
	} else if (name == "tutorial0002") {
		UI::get().push("intropathfinding");
	} else if (name == "tutorial0003") {
		UI::get().push("introdeadlock");
	}
}

bool UIMapWindow::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	if (IUIMapWindow::onFingerMotion(finger, x, y, dx, dy))
		return true;
	_nodeMap->getMap().scroll(dx, dy);
	return true;
}

bool UIMapWindow::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	if (!IUIMapWindow::onMouseButtonRelease(x, y, button)) {
		_scrolling = false;
	}
	return true;
}

bool UIMapWindow::getField (int32_t x, int32_t y, int *tx, int *ty) const
{
	_nodeMap->getMap().getMapGridForScreenPixel(x, y, tx, ty);
	if (*tx == -1 || *ty == -1)
		return false;
	return true;
}

bool UIMapWindow::onFingerRelease (int64_t finger, uint16_t x, uint16_t y, bool motion)
{
	const bool retVal = IUIMapWindow::onFingerRelease(finger, x, y, motion);
	if (!retVal)
		tryMove(x, y, false);
	return retVal;
}

void UIMapWindow::doMove (int tx, int ty)
{
	Log::debug(LOG_GAMEIMPL, "send move message to reach %i:%i", tx, ty);
	_serviceProvider.getNetwork().sendToServer(MoveToMessage(tx, ty));
	_targetX = _targetY = -1;
}

bool UIMapWindow::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	const bool retVal = IUIMapWindow::onMouseButtonPress(x, y, button);
	if (!retVal) {
		_scrolling = true;
	}
	if (button == SDL_BUTTON_LEFT) {
		// double clicking onto the same field means, that the users wanna walk there
		if (tryMove(x, y, true))
			return true;
	}

	return retVal;
}

bool UIMapWindow::tryMove (int x, int y, bool doubleTap)
{
	int tx, ty;
	if (getField(x, y, &tx, &ty)) {
		Log::debug(LOG_GAMEIMPL, "resolved the grid coordinates for %i:%i to %i:%i", x, y, tx, ty);
		if (!doubleTap) {
			_targetX = tx;
			_targetY = ty;
		}
		if (_targetX == tx && _targetY == ty) {
			doMove(_targetX, _targetY);
		} else {
			_targetX = tx;
			_targetY = ty;
		}
		return true;
	}
	Log::debug(LOG_GAMEIMPL, "could not get grid coordinates for %i:%i", x, y);
	return false;
}

void UIMapWindow::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	if (_scrolling) {
		_nodeMap->getMap().scroll(relX, relY);
	}
	IUIMapWindow::onMouseMotion(x, y, relX, relY);
}

}
