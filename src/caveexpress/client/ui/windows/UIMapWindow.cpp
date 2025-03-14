#include "UIMapWindow.h"
#include "caveexpress/client/ui/nodes/UINodeMap.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "client/IMapControl.h"
#include "common/Math.h"
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
	_points->setLabel("0");
	_points->setId(UINODE_POINTS);
	_panel->add(_points);

	//  time  ---
	UINodeBar* timeBar = new UINodeBar(_frontend);
	timeBar->setId(UINODE_SECONDS_BAR);
	const Color timeBarColor = { 1.0f, 1.0f, 1.0f, 0.5f };
	timeBar->setSize(barWidth, barHeight);
	timeBar->setBarColor(timeBarColor);
	timeBar->setBorder(true);
	timeBar->setBorderColor(colorWhite);
	_panel->add(timeBar);

	UINodeLabel *timeLeft = new UINodeLabel(_frontend, "1:00");
	timeLeft->setId(UINODE_SECONDS_REMAINING);
	timeLeft->setFont(HUGE_FONT);
	timeLeft->setColor(colorWhite);
	_panel->add(timeLeft);

	//  hp bar  ---
	UINodeBar* hitpointsBar = new UINodeBar(_frontend);
	hitpointsBar->setId(UINODE_HITPOINTS);
	const int maxHitpoints = Config.getConfigVar(MAX_HITPOINTS)->getIntValue();
	hitpointsBar->setMax(maxHitpoints);
	hitpointsBar->setSize(barWidth, barHeight);
	hitpointsBar->setBorder(true);
	hitpointsBar->setBorderColor(colorWhite);
	// TODO: wind indicator / wind particles
	_panel->add(hitpointsBar);

	//  lives
	UINodeSprite* livesSprite = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	livesSprite->setId(UINODE_LIVES);
	livesSprite->setSpriteOffset(spriteHeight);
	const SpritePtr sprite = UI::get().loadSprite("icon-heart");
	for (uint8_t i = 0; i < INITIAL_LIVES; ++i) {
		livesSprite->addSprite(sprite);
	}
	_panel->add(livesSprite);


	//  target cave  ---
	UINodeSprite *targetCave = new UINodeSprite(_frontend, spriteHeight * 2, spriteHeight);
	targetCave->setId(UINODE_TARGETCAVEID);
	targetCave->setImage("icon-targetcave");
	_panel->add(targetCave);

	UINodeSprite *collected = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	collected->setId(UINODE_COLLECTED);
	_panel->add(collected);

	UINodeLabel *spacer1 = new UINodeLabel(_frontend, "    ");
	spacer1->setFont(HUGE_FONT);
	spacer1->setVisible(false);
	_panel->add(spacer1);

	
	//  transfers  ---
	UINodeSprite *npcIcon = new UINodeSprite(_frontend, spriteHeight*3/2, spriteHeight);
	npcIcon->setSpriteOffset(spriteNodeOffset / 2);
	collected->setId(UINODE_TRANSFERS);
	const std::string npcName = SpriteDefinition::get().getSpriteName(
		EntityTypes::NPC_FRIENDLY_MAN, Animations::ANIMATION_IDLE);
	const SpritePtr npcSprite = UI::get().loadSprite(npcName);
	npcIcon->addSprite(npcSprite);
	_panel->add(npcIcon);

	UINodeLabel *npcLeft = new UINodeLabel(_frontend, "");
	npcLeft->setId(UINODE_TRANSFERS_LEFT);
	npcLeft->setFont(HUGE_FONT);
	npcLeft->setColor(colorWhite);
	_panel->add(npcLeft);

	UINodeLabel *spacer2 = new UINodeLabel(_frontend, "    ");
	spacer2->setFont(HUGE_FONT);
	spacer2->setVisible(false);
	_panel->add(spacer2);

	//  packages  ---
	UINodeSprite* packageIcon = new UINodeSprite(_frontend, spriteHeight, spriteHeight);
	packageIcon->setId(UINODE_PACKAGES);
	const std::string pkgName = SpriteDefinition::get().getSpriteName(
		EntityTypes::PACKAGE_ROCK, Animations::ANIMATION_IDLE);
	const SpritePtr pkgIco = UI::get().loadSprite(pkgName);
	packageIcon->addSprite(pkgIco);
	_panel->add(packageIcon);

	UINodeLabel *pkgLeft = new UINodeLabel(_frontend, "");
	pkgLeft->setId(UINODE_PACKAGES_LEFT);
	pkgLeft->setFont(HUGE_FONT);
	pkgLeft->setColor(colorWhite);
	_panel->add(pkgLeft);

	add(_panel);
}

}
