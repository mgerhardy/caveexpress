#include "Intro1.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/client/ui/UI.h"

Intro1::Intro1(IFrontend* frontend) :
		Intro("intro1", frontend) {
	init();
}

void Intro1::addIntroNodes(UINode* parent) {
#if 0
	const std::string spriteName = SpriteDefinition::get().getSpriteName(EntityTypes::PLAYER, Animations::ANIMATION_IDLE);
	const SpritePtr& spritePtr = UI::get().loadSprite(spriteName);
	UINodeSprite* spriteNode = new UINodeSprite(_frontend);
	spriteNode->addSprite(spritePtr);
	const float h = parent->getHeight();
	spriteNode->setSize(h, h);
	spriteNode->setAspectRatioSize(h, h, 1.3f);
	spriteNode->alignToMiddle();
	parent->add(spriteNode);
#endif
}
