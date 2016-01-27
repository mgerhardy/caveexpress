#include "UIMapWindow.h"
#include "caveexpress/client/ui/nodes/UINodeMap.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "client/IMapControl.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeButtonText.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/windows/listener/OpenWindowListener.h"
#include "common/ConfigManager.h"
#include "campaign/persister/IGameStatePersister.h"

namespace caveexpress {

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, ClientMap& map) :
		IUIMapWindow(frontend, serviceProvider, campaignManager, new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0, frontend->getWidth(), frontend->getHeight(), map), true)
{
	init();
}

void UIMapWindow::initHudNodes()
{
	const float barHeight = 20.0f / _frontend->getHeight();
	const int spriteHeight = 50;
	const float barWidth = 200.0f / _frontend->getWidth();
	const int spriteNodeOffset = 40;

	_panel = new UINode(_frontend);
	_panel->setId("hudpanel");
	UIHBoxLayout* layout = new UIHBoxLayout();
	layout->setSpacing(0.02f);
	_panel->setLayout(layout);
	_panel->setStandardPadding();
	_panel->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_CENTER);

	UINodePoint* _points = new UINodePoint(_frontend, 150);
	_points->setLabel("00000");
	_points->setId(UINODE_POINTS);
	_panel->add(_points);

	UINodeBar* timeBar = new UINodeBar(_frontend);
	timeBar->setId(UINODE_SECONDS_REMAINING);
	const Color timeBarColor = { 1.0f, 1.0f, 1.0f, 0.5f };
	timeBar->setSize(barWidth, barHeight);
	timeBar->setBarColor(timeBarColor);
	timeBar->setBorder(true);
	timeBar->setBorderColor(colorWhite);
	_panel->add(timeBar);

	UINodeBar* hitpointsBar = new UINodeBar(_frontend);
	hitpointsBar->setId(UINODE_HITPOINTS);
	const int maxHitpoints = Config.getConfigVar(MAX_HITPOINTS)->getIntValue();
	hitpointsBar->setMax(maxHitpoints);
	hitpointsBar->setSize(barWidth, barHeight);
	hitpointsBar->setBorder(true);
	hitpointsBar->setBorderColor(colorWhite);
	// TODO: wind indicator

	_panel->add(hitpointsBar);

	UINodeSprite* livesSprite = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	livesSprite->setId(UINODE_LIVES);
	livesSprite->setSpriteOffset(spriteHeight);
	const SpritePtr sprite = UI::get().loadSprite("icon-heart");
	for (uint8_t i = 0; i < INITIAL_LIVES; ++i) {
		livesSprite->addSprite(sprite);
	}
	_panel->add(livesSprite);

	UINodeSprite *targetCave = new UINodeSprite(_frontend, spriteHeight * 2, spriteHeight);
	targetCave->setId(UINODE_TARGETCAVEID);
	targetCave->setImage("icon-targetcave");
	_panel->add(targetCave);

	UINodeSprite *collected = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	collected->setId(UINODE_COLLECTED);
	_panel->add(collected);

	UINodeSprite* packagesSprite = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	packagesSprite->setId(UINODE_PACKAGES);
	packagesSprite->setSpriteOffset(spriteNodeOffset);
	_panel->add(packagesSprite);

	add(_panel);
}

}
