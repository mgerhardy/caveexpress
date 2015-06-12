#include "IntroFindYourWay.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "ui/UI.h"

namespace caveexpress {

IntroFindYourWay::IntroFindYourWay(IFrontend* frontend) :
		Intro("introfindyourway", frontend) {
	init();
}

void IntroFindYourWay::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Try to avoid crashes but still be fast")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Hitting walls hard will inflict damage")));
	parent->add(new IntroLabel(_frontend, tr("Collecting fruits will restore hitpoints")));
	const Color timeBarColor = { 1.0f, 1.0f, 1.0f, 0.5f };
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroBarDescription(_frontend, timeBarColor, tr("Time bar")));
	parent->add(new IntroBarDescription(_frontend, tr("Health bar")));
}

}
