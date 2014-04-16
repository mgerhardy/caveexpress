#include "IntroFlying.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "engine/client/ui/UI.h"

IntroFlying::IntroFlying(IFrontend* frontend) :
		Intro("introflying", frontend) {
	init();
}

void IntroFlying::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Just stay and watch what happens")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Dropping stones onto the dinos will stun them")));
	if (System.hasTouch() && !System.isOUYA()) {
		parent->add(new IntroLabel(_frontend, tr("Drop them with the second finger")));
	} else {
		parent->add(new IntroLabel(_frontend, tr("Drop them by hitting SPACE bar")));
	}
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(_frontend, EntityTypes::NPC_FLYING, Animations::ANIMATION_FLYING_RIGHT, tr("Pterodactylus")));
	parent->add(new IntroTypeDescription(_frontend, EntityTypes::STONE, Animations::ANIMATION_IDLE, tr("Stone")));
}
