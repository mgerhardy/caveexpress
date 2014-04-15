#include "IntroGeyser.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "engine/client/ui/UI.h"

IntroGeyser::IntroGeyser(IFrontend* frontend) :
		Intro("introgeyser", frontend) {
	init();
}

void IntroGeyser::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Watch the geyser")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("The geyser will do the work for you")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(_frontend, EntityTypes::GEYSER_ROCK, Animations::ANIMATION_ACTIVE, tr("Geyser")));
}
