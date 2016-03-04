#include "IUIMapWindow.h"
#include "ui/UI.h"
#include "client/IMapControl.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeButtonText.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/windows/listener/OpenWindowListener.h"
#include "common/IFrontend.h"
#include "common/System.h"
#include "common/Commands.h"
#include "common/CommandSystem.h"
#include "common/ConfigManager.h"
#include "ui/windows/UIWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "ui/nodes/IUINodeMap.h"
#include "ui/nodes/UINodeMapControl.h"
#include "ui/nodes/UINodeMapFingerControl.h"
#include "campaign/persister/IGameStatePersister.h"
#include <SDL_platform.h>

class UINodeSettingsButton: public UINodeButton {
private:
	IMapControl *_mapControl;
public:
	UINodeSettingsButton (IFrontend *frontend, IMapControl *mapControl) :
			UINodeButton(frontend), _mapControl(mapControl)
	{
	}

	bool isActive () const override
	{
		if (_mapControl != nullptr)
			return !_mapControl->isPressed();
		return true;
	}
};

IUIMapWindow::IUIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, IUINodeMap* nodeMap, bool continuousMovement) :
		UIWindow(UI_WINDOW_MAP, frontend, WINDOW_FLAG_MODAL | WINDOW_FLAG_FULLSCREEN), _nodeMap(nodeMap), _waitLabel(nullptr), _mapControl(nullptr),
		_startButton(nullptr), _serviceProvider(serviceProvider), _panel(nullptr), _lastFingerPressEvent(0L), _cursorActive(false), _continuousMovement(continuousMovement) {
	const float screenPadding = getScreenPadding();
	setPadding(screenPadding);
	_playClickSound = false;
	_musicFile = "music-2";
	_onPop = CMD_CL_DISCONNECT;
	_nodeMap->setId(UINODE_MAP);
	add(_nodeMap);
}

void IUIMapWindow::hideHud()
{
	if (_panel)
		_panel->setVisible(false);
	if (_mapControl)
		_mapControl->hide();
}

void IUIMapWindow::showHud()
{
	if (_panel)
		_panel->setVisible(true);
	if (_mapControl)
		_mapControl->show();
}

void IUIMapWindow::initInputHudNodes()
{
	UINodeSettingsButton *settings = new UINodeSettingsButton(_frontend, _mapControl);
	settings->setImage("icon-settings");
	settings->setId("settings");
	settings->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_OPTIONS)));
	settings->setAlignment(NODE_ALIGN_LEFT | NODE_ALIGN_TOP);
	add(settings);
}

void IUIMapWindow::initHudNodes()
{
}

void IUIMapWindow::init()
{
	initHudNodes();
	if (Config.getConfigVar("forcefingercontrol", "false", true, CV_READONLY)->getBoolValue() || System.hasTouch()) {
		UINode* node = getFingerControl();
		if (node != nullptr)
			add(node);
	} else {
		UINode* node = getControl();
		if (node != nullptr)
			add(node);
	}

	initInputHudNodes();

	_startButton = new UINodeButtonText(_frontend, tr("Start"), 0.05f);
	_startButton->setId("startbutton");
	_startButton->setOnActivate(CMD_START);
	_startButton->setFont(getFont(LARGE_FONT), colorBlack);
	_startButton->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	_startButton->setVisible(false);
	add(_startButton);

	_waitLabel = new UINodeLabel(_frontend, tr("Waiting"), getFont(LARGE_FONT));
	_waitLabel->setId("waitpanel");
	_waitLabel->setColor(colorWhite);
	_waitLabel->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	_waitLabel->setVisible(false);
	add(_waitLabel);
}

UINode* IUIMapWindow::getControl ()
{
	UINodeMapControl* node = new UINodeMapControl(_frontend, _nodeMap, _continuousMovement);
	_mapControl = node;
	return node;
}

UINode* IUIMapWindow::getFingerControl ()
{
	UINodeMapFingerControl* node = new UINodeMapFingerControl(_frontend, _nodeMap);
	_mapControl = node;
	return node;
}

void IUIMapWindow::onActive ()
{
	UIWindow::onActive();
	_cursorActive = UI::get().isCursorVisible();
	if (_cursorActive && !_startButton->isVisible())
		showCursor(false);
	UINode* lives = getNode(UINODE_LIVES);
	if (lives != nullptr)
		lives->setVisible(Config.isModeHard());

	//if (!getSystem().hasItem(PAYMENT_ADFREE)) {
	//	const int h = _frontend->getHeight() - getSystem().getAdHeight();
	//	_nodeMap->setMapRect(0, 0, _frontend->getWidth(), h);
	//}

	if (_nodeMap->getMap().isStarted())
		Config.setBindingsSpace(BINDINGS_MAP);
}

void IUIMapWindow::onPushedOver ()
{
	showHud();
	UIWindow::onPushedOver();
	if (_cursorActive)
		showCursor(true);
}

bool IUIMapWindow::onPop ()
{
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		(*i)->removeFocus(FOCUS_POP);
	}
	Config.setBindingsSpace(BINDINGS_UI);
	if (_cursorActive)
		showCursor(true);
	if (!UI::get().isMainRoot()) {
		// editor
		Commands.executeCommandLine(CMD_CL_DISCONNECT);
		return UIWindow::onPop();
	}
	if (!_nodeMap->getMap().isActive()) {
		return UIWindow::onPop();
	}
	UI::get().push(UI_WINDOW_OPTIONS);
	return false;
}

void IUIMapWindow::initWaitingForPlayers (bool adminOptions)
{
	if (_nodeMap->initWaitingForPlayer()) {
		return;
	}

	if (adminOptions) {
		_startButton->setVisible(true);
		_waitLabel->setVisible(false);
		// we need the cursor back to click onto the start button
		if (_cursorActive)
			showCursor(true);
	} else {
		_startButton->setVisible(false);
		_waitLabel->setVisible(true);
	}
}

void IUIMapWindow::showCursor (bool show)
{
	UI::get().showCursor(show);
}

void IUIMapWindow::start ()
{
	_startButton->setVisible(false);
	_waitLabel->setVisible(false);
	if (_cursorActive)
		showCursor(false);
	_nodeMap->start();
	Config.setBindingsSpace(BINDINGS_MAP);
}

bool IUIMapWindow::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	_lastFingerPressEvent = _time;
	return UIWindow::onFingerPress(finger, x, y);
}

bool IUIMapWindow::onMultiGesture (float theta, float dist, int32_t numFingers)
{
	const bool retVal = UIWindow::onMultiGesture(theta, dist, numFingers);
	if (numFingers == 2 && _time - _lastFingerPressEvent > 500L) {
		const float currentZoom = _nodeMap->getMap().getZoom();
		_nodeMap->getMap().setZoom(currentZoom + dist * 4.0f);
	}
	return retVal;
}

bool IUIMapWindow::onMouseWheel (int32_t x, int32_t y)
{
	const float currentZoom = _nodeMap->getMap().getZoom();
	const float step = 0.1f;
	if (y > 0)
		_nodeMap->getMap().setZoom(currentZoom + step);
	else
		_nodeMap->getMap().setZoom(currentZoom - step);
	return UIWindow::onMouseWheel(x, y);
}
