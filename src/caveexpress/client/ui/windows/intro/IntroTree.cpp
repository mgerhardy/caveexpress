#include "IntroTree.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "ui/UI.h"

namespace caveexpress {

IntroTree::IntroTree(IFrontend* frontend) :
		Intro("introtree", frontend) {
	init();
}

void IntroTree::addIntroNodes(UINode* parent) {
	parent->add(new IntroLabelHeadline(_frontend, tr("Objectives")));
	parent->add(new IntroLabel(_frontend, tr("Learn to use the stone")));
	parent->add(new IntroLabelHeadline(_frontend, tr("Hints")));
	parent->add(new IntroLabel(_frontend, tr("Drop stones onto the tree")));
	if (System.hasTouch()) {
		parent->add(new IntroLabel(_frontend, tr("Drop them with the second finger")));
	} else {
		parent->add(new IntroLabel(_frontend, tr("Drop them by hitting SPACE bar")));
	}
	parent->add(new IntroLabelHeadline(_frontend, tr("Description")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::TREE, Animations::ANIMATION_IDLE, tr("Tree")));
	parent->add(new IntroTypeDescription(parent, _frontend, EntityTypes::STONE, Animations::ANIMATION_IDLE, tr("Stone")));
}

}
