#include "IntroLava.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "ui/UI.h"

namespace caveexpress {

IntroLava::IntroLava(IFrontend* frontend) :
		Intro("introlava", frontend) {
	init();
}

void IntroLava::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Deliver the packages without touching lava")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Fly over the lava - do not land in it")));
	parent->add(new IntroLabel(_frontend, tr("Lava destroys your plane on contact")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::LAVA, Animations::ANIMATION_IDLE, tr("Lava")));
}

}
