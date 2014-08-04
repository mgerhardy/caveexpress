#include "IUIMapWindow.h"
#include "engine/client/ui/UI.h"
#include "engine/client/IMapControl.h"
#include "engine/client/ui/nodes/UINodeButton.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodePoint.h"
#include "engine/client/ui/nodes/UINodeButtonText.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"
#include "engine/client/ui/windows/listener/OpenWindowListener.h"
#include "engine/common/IFrontend.h"
#include "engine/common/System.h"
#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/ConfigManager.h"
#include "engine/client/ui/windows/UIWindow.h"
#include "engine/client/ui/nodes/IUINodeMap.h"
#include "engine/client/ui/nodes/IUINodeMap.h"
#include "engine/client/ui/nodes/UINodeMapControl.h"
#include "engine/client/ui/nodes/UINodeMapFingerControl.h"
#include "engine/common/campaign/persister/IGameStatePersister.h"
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
		return !_mapControl->isPressed();
	}
};

IUIMapWindow::IUIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map, IUINodeMap* nodeMap) :
		UIWindow(UI_WINDOW_MAP, frontend,
				WINDOW_FLAG_MODAL | WINDOW_FLAG_FULLSCREEN), _nodeMap(nodeMap), _cursorActive(
				false), _serviceProvider(serviceProvider), _startButton(
				nullptr), _waitLabel(nullptr), _mapControl(nullptr), _panel(nullptr) {
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
	settings->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_OPTIONS)));
	settings->setAlignment(NODE_ALIGN_LEFT | NODE_ALIGN_TOP);
	add(settings);
}

void IUIMapWindow::initHudNodes()
{
	const float barHeight = 12.0f / _frontend->getHeight();
	const int spriteHeight = 30;
	const float barWidth = 102.0f / _frontend->getWidth();
	const int spriteNodeOffset = 15;

	_panel = new UINode(_frontend);
	UIHBoxLayout* layout = new UIHBoxLayout();
	layout->setSpacing(0.02f);
	_panel->setLayout(layout);
	_panel->setStandardPadding();
	_panel->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_CENTER);

	UINodePoint* _points = new UINodePoint(_frontend, 150);
	_points->setLabel("00000");
	_points->setId(UINODE_POINTS);
	_panel->add(_points);

	UINodeBar* _timeBar = new UINodeBar(_frontend);
	_timeBar->setId(UINODE_SECONDS_REMAINING);
	const Color timeBarColor = { 1.0f, 1.0f, 1.0f, 0.5f };
	_timeBar->setSize(barWidth, barHeight);
	_timeBar->setBarColor(timeBarColor);
	_timeBar->setBorder(true);
	_timeBar->setBorderColor(colorWhite);
	_panel->add(_timeBar);

	UINodeBar* _hitpointsBar = new UINodeBar(_frontend);
	_hitpointsBar->setId(UINODE_HITPOINTS);
	const int maxHitpoints = Config.getMaxHitpoints();
	_hitpointsBar->setMax(maxHitpoints);
	_hitpointsBar->setSize(barWidth, barHeight);
	_hitpointsBar->setBorder(true);
	_hitpointsBar->setBorderColor(colorWhite);
	// TODO: wind indicator

	_panel->add(_hitpointsBar);

	UINodeSprite* _livesSprite = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	_livesSprite->setId(UINODE_LIVES);
	_livesSprite->setSpriteOffset(spriteHeight);
	const SpritePtr sprite = UI::get().loadSprite("icon-heart");
	for (uint8_t i = 0; i < INITIAL_LIVES; ++i) {
		_livesSprite->addSprite(sprite);
	}
	_panel->add(_livesSprite);

	UINodeSprite *collected = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	collected->setId(UINODE_COLLECTED);
	_panel->add(collected);

	UINodeSprite* _packagesSprite = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	_packagesSprite->setId(UINODE_PACKAGES);
	_packagesSprite->setSpriteOffset(spriteNodeOffset);
	_panel->add(_packagesSprite);

	add(_panel);
}

void IUIMapWindow::init()
{
	initHudNodes();
	if (System.hasTouch()) {
		UINode* node = getFingerControl();
		add(node);
	} else {
		UINode* node = getControl();
		add(node);
	}

	initInputHudNodes();

	_startButton = new UINodeButtonText(_frontend, tr("Start"), 0.05f);
	_startButton->setOnActivate(CMD_START);
	_startButton->setFont(getFont(LARGE_FONT), colorBlack);
	_startButton->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	_startButton->setVisible(false);
	add(_startButton);

	_waitLabel = new UINodeLabel(_frontend, tr("Waiting"), getFont(LARGE_FONT));
	_waitLabel->setColor(colorWhite);
	_waitLabel->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	_waitLabel->setVisible(false);
	add(_waitLabel);
}

UINode* IUIMapWindow::getControl ()
{
	UINodeMapControl* node = new UINodeMapControl(_frontend, _nodeMap);
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
		(*i)->removeFocus();
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

bool IUIMapWindow::isGameActive () const
{
	return _nodeMap->getMap().isActive();
}
