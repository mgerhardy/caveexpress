#include "IntroAttack.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "ui/UI.h"

namespace caveexpress {

IntroAttack::IntroAttack(IFrontend* frontend) :
		Intro("introattack", frontend) {
	init();
}

void IntroAttack::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Just stay and watch what happens")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("The egg makes you invulverable")));
	parent->add(new IntroLabel(_frontend, tr("Dropping stones onto dinos stuns them")));
	if (System.hasTouch()) {
		parent->add(new IntroLabel(_frontend, tr("Drop them with the second finger")));
	} else {
		parent->add(new IntroLabel(_frontend, tr("Drop them by hitting SPACE bar")));
		parent->add(new IntroLabel(_frontend, tr("or A on your controller")));
	}
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::NPC_WALKING, Animations::ANIMATION_ATTACK_INIT_RIGHT, tr("Angry dinosaur")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::STONE, Animations::ANIMATION_IDLE, tr("Stone")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::EGG, Animations::ANIMATION_IDLE, tr("Egg")));
}

}
