#include "IntroGame.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "ui/UI.h"

IntroGame::IntroGame(IFrontend* frontend) :
		Intro("introgame", frontend) {
	init();
}

void IntroGame::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Move all packages onto their targets")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("The less moves you do the better")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PLAYER, Animation::NONE, tr("Your player")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::PACKAGE, Animation::NONE, tr("Package")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::TARGET, Animation::NONE, tr("Target")));
}
