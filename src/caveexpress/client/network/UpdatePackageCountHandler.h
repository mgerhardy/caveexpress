#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdatePackageCountMessage.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/windows/IUIMapWindow.h"
#include <string>

namespace caveexpress {

class UpdatePackageCountHandler: public ClientProtocolHandler<UpdatePackageCountMessage> {
public:
	void execute (const UpdatePackageCountMessage* msg) override
	{
		const uint8_t packages = msg->getPackages();
		const uint8_t packagesNeeded = msg->getPackagesNeeded();
		
		/*node->clearSprites();
		const std::string name = SpriteDefinition::get().getSpriteName(EntityTypes::PACKAGE_ROCK,
				Animations::ANIMATION_IDLE);
		const SpritePtr sprite = UI::get().loadSprite(name);
		// for (uint8_t i = 0; i < packages; ++i) {
			node->addSprite(sprite);
		// }
		// node->flash();
		*/
		UINodeSprite* packageIcon = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_PACKAGES);
		packageIcon->flash();
		
		UINodeLabel *pkgLeft = UI::get().getNode<UINodeLabel>(UI_WINDOW_MAP, UINODE_PACKAGES_LEFT);
		pkgLeft->setLabel(string::format("%d/%d", packages, packagesNeeded));
	}
};

}
