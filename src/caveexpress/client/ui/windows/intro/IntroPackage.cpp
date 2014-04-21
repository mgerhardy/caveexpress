#include "IntroPackage.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/layouts/UIHBoxLayout.h"

IntroPackage::IntroPackage(IFrontend* frontend) :
		Intro("intropackage", frontend) {
	init();
}

void IntroPackage::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Deliver packages to the shredders")));
	parent->add(new IntroLabel(_frontend, tr("This is the goal in every map")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Collect packages by landing on them")));
	if (System.hasTouch() && !System.isOUYA()) {
		parent->add(new IntroLabel(_frontend, tr("Drop them with the second finger")));
	} else {
		parent->add(new IntroLabel(_frontend, tr("Drop them by hitting SPACE bar")));
	}
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	UINode* panel = new UINode(_frontend);
	panel->setLayout(new UIHBoxLayout(0.01f));
	panel->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PLAYER, Animations::ANIMATION_FLYING, tr("Player")));
	panel->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PACKAGE_ROCK, Animations::ANIMATION_IDLE, tr("Package")));
	parent->add(panel);
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PACKAGETARGET_ROCK, Animations::ANIMATION_ROTATE, tr("Shredder")));
}
