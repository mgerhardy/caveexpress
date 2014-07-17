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
#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/ConfigManager.h"
#include "engine/client/ui/nodes/IUINodeMap.h"
#include "engine/client/ui/nodes/IUINodeMap.h"
#include "engine/client/ui/nodes/UINodeMapControl.h"
#include "engine/client/ui/nodes/UINodeMapFingerControl.h"
#include "engine/common/campaign/persister/IGameStatePersister.h"
#include "caveexpress/shared/constants/Commands.h"
#include "caveexpress/shared/constants/PlayerState.h"
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
		UIWindow(UI_WINDOW_MAP, frontend, WINDOW_FLAG_MODAL | WINDOW_FLAG_FULLSCREEN), _nodeMap(nodeMap), _cursorActive(false), _serviceProvider(serviceProvider)
{
	const float screenPadding = getScreenPadding();
	setPadding(screenPadding);
	_playClickSound = false;
	_musicFile = "music-2";
	_onPop = CMD_CL_DISCONNECT;
	_nodeMap->setId(UINODE_MAP);
	add(_nodeMap);

	const float barHeight = 12.0f / _frontend->getHeight();
	const int spriteHeight = 30;
	const float barWidth = 102.0f / _frontend->getWidth();
	const int spriteNodeOffset = 15;

	_panel = new UINode(frontend);
	UIHBoxLayout* layout = new UIHBoxLayout();
	layout->setSpacing(0.02f);
	_panel->setLayout(layout);
	_panel->setStandardPadding();
	_panel->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_CENTER);

	_points = new UINodePoint(frontend, 150);
	_points->setLabel("00000");
	_points->setId(UINODE_POINTS);
	_panel->add(_points);

	_timeBar = new UINodeBar(frontend);
	_timeBar->setId(UINODE_SECONDS_REMAINING);
	const Color timeBarColor = { 1.0f, 1.0f, 1.0f, 0.5f };
	_timeBar->setSize(barWidth, barHeight);
	_timeBar->setBarColor(timeBarColor);
	_timeBar->setBorder(true);
	_timeBar->setBorderColor(colorWhite);
	_panel->add(_timeBar);

	_hitpointsBar = new UINodeBar(frontend);
	_hitpointsBar->setId(UINODE_HITPOINTS);
	const int maxHitpoints = Config.getMaxHitpoints();
	_hitpointsBar->setMax(maxHitpoints);
	_hitpointsBar->setSize(barWidth, barHeight);
	_hitpointsBar->setBorder(true);
	_hitpointsBar->setBorderColor(colorWhite);
	// TODO: wind indicator

	_panel->add(_hitpointsBar);

	_livesSprite = new UINodeSprite(frontend, spriteHeight, spriteHeight);
	_livesSprite->setId(UINODE_LIVES);
	_livesSprite->setSpriteOffset(spriteHeight);
	const SpritePtr sprite = UI::get().loadSprite("icon-heart");
	for (uint8_t i = 0; i < INITIAL_LIVES; ++i) {
		_livesSprite->addSprite(sprite);
	}
	_panel->add(_livesSprite);

	UINodeSprite *collected = new UINodeSprite(frontend, spriteHeight, spriteHeight);
	collected->setId(UINODE_COLLECTED);
	_panel->add(collected);

	_packagesSprite = new UINodeSprite(frontend, spriteHeight, spriteHeight);
	_packagesSprite->setId(UINODE_PACKAGES);
	_packagesSprite->setSpriteOffset(spriteNodeOffset);
	_panel->add(_packagesSprite);

	add(_panel);

	if (System.hasTouch()) {
		UINodeMapFingerControl* node = new UINodeMapFingerControl(frontend, _nodeMap);
		_mapControl = node;
		add(node);
	} else {
		UINodeMapControl* node = new UINodeMapControl(frontend, _nodeMap);
		_mapControl = node;
		add(node);
	}

	if (!System.hasTouch()) {
		UINodeSettingsButton *settings = new UINodeSettingsButton(frontend, _mapControl);
		settings->setImage("icon-settings");
		settings->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_OPTIONS)));
		settings->setAlignment(NODE_ALIGN_LEFT | NODE_ALIGN_TOP);
		add(settings);
	}

	_startButton = new UINodeButtonText(frontend, tr("Start"), 0.05f);
	_startButton->setOnActivate(CMD_START);
	_startButton->setFont(getFont(LARGE_FONT), colorBlack);
	_startButton->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	_startButton->setVisible(false);
	add(_startButton);

	_waitLabel = new UINodeLabel(frontend, tr("Waiting"), getFont(LARGE_FONT));
	_waitLabel->setColor(colorWhite);
	_waitLabel->setAlignment(NODE_ALIGN_CENTER | NODE_ALIGN_TOP);
	_waitLabel->setVisible(false);
	add(_waitLabel);
}

void IUIMapWindow::onActive ()
{
	UIWindow::onActive();
	_cursorActive = UI::get().isCursorVisible();
	if (_cursorActive && !_startButton->isVisible())
		UI::get().showCursor(false);
	_livesSprite->setVisible(Config.isModeHard());

	//if (!getSystem().hasItem(PAYMENT_ADFREE)) {
	//	const int h = _frontend->getHeight() - getSystem().getAdHeight();
	//	_nodeMap->setMapRect(0, 0, _frontend->getWidth(), h);
	//}

	if (_nodeMap->getMap().isStarted())
		Config.setBindingsSpace(BINDINGS_MAP);
}

void IUIMapWindow::onPushedOver ()
{
	UIWindow::onPushedOver();
	if (_cursorActive)
		UI::get().showCursor(true);
}

bool IUIMapWindow::onPop ()
{
	for (UINodeListConstIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		(*i)->removeFocus();
	}
	Config.setBindingsSpace(BINDINGS_UI);
	if (_cursorActive)
		UI::get().showCursor(true);
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
			UI::get().showCursor(true);
	} else {
		_startButton->setVisible(false);
		_waitLabel->setVisible(true);
	}
}

void IUIMapWindow::start ()
{
	_startButton->setVisible(false);
	_waitLabel->setVisible(false);
	if (_cursorActive)
		UI::get().showCursor(false);
	_nodeMap->start();
	Config.setBindingsSpace(BINDINGS_MAP);
}

bool IUIMapWindow::isGameActive () const
{
	return _nodeMap->getMap().isActive();
}
