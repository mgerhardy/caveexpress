#include "IntroDeadlock.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "cavepacker/shared/CavePackerAnimation.h"
#include "ui/UI.h"

namespace cavepacker {

IntroDeadlock::IntroDeadlock(IFrontend* frontend) :
		Intro("introdeadlock", frontend) {
	init();
}

void IntroDeadlock::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Move all packages onto their targets")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Red packages are deadlocks - undo your step")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PLAYER, Animation::NONE, tr("Your player")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PACKAGE, Animations::DEADLOCK, tr("Deadlock package")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::TARGET, Animation::NONE, tr("Target")));
}

}
