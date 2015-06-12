#include "IntroFlying.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "ui/UI.h"

namespace caveexpress {

IntroFlying::IntroFlying(IFrontend* frontend) :
		Intro("introflying", frontend) {
	init();
}

void IntroFlying::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Just stay and watch what happens")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Dropping stones onto dinos stuns them")));
	if (System.hasTouch() && !System.isOUYA()) {
		parent->add(new IntroLabel(_frontend, tr("Drop them with the second finger")));
	} else {
		parent->add(new IntroLabel(_frontend, tr("Drop them by hitting SPACE bar")));
	}
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::NPC_FLYING, Animations::ANIMATION_FLYING_RIGHT, tr("Pterodactyls")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::STONE, Animations::ANIMATION_IDLE, tr("Stone")));
}

}
