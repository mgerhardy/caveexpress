#include "IntroDiving.h"
#include "ui/UI.h"

namespace caveexpress {

IntroDiving::IntroDiving(IFrontend* frontend) :
		Intro("introdiving", frontend) {
	init();
}

void IntroDiving::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("To dive across, fall fast into water flying to side")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Hitting walls hard will inflict damage")));
	const Color timeBarColor = { 1.0f, 1.0f, 1.0f, 0.5f };
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroBarDescription(_frontend, timeBarColor, tr("Time bar")));
	parent->add(new IntroBarDescription(_frontend, tr("Health bar")));
}

}
