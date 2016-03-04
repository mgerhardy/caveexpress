#include "IntroPathfinding.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "ui/UI.h"

namespace cavepacker {

IntroPathfinding::IntroPathfinding(IFrontend* frontend) :
		Intro("intropathfinding", frontend) {
	init();
}

void IntroPathfinding::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Move all packages onto their targets")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Clicking the position will move the player there")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PLAYER, Animation::NONE, tr("Your player")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PACKAGE, Animation::NONE, tr("Package")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::TARGET, Animation::NONE, tr("Target")));
}

}
