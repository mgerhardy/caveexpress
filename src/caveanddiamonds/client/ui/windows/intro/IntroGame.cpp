#include "IntroGame.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "caveanddiamonds/shared/CaveAndDiamondsEntityType.h"
#include "engine/client/ui/UI.h"

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
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::STONE, Animation::NONE, tr("Stone")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::TARGET, Animation::NONE, tr("Target")));
}
