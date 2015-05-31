#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdateLivesMessage.h"
#include "client/ui/UI.h"
#include "client/ui/windows/IUIMapWindow.h"
#include "common/campaign/CampaignManager.h"

class UpdateLivesHandler: public IClientProtocolHandler {
private:
	CampaignManager& _campaignManager;
public:
	UpdateLivesHandler (CampaignManager& campaignManager) :
			_campaignManager(campaignManager)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const UpdateLivesMessage *msg = static_cast<const UpdateLivesMessage*>(&message);
		const uint8_t lives = msg->getLives();
		CampaignPtr campaign = _campaignManager.getActiveCampaign();
		if (campaign) {
			campaign->setLives(lives);
			campaign->saveProgress();
		}
		UINodeSprite* node = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_LIVES);
		if (!node)
			return;
		node->clearSprites();
		const SpritePtr sprite = UI::get().loadSprite("icon-heart");
		for (uint8_t i = 0; i < lives; ++i) {
			node->addSprite(sprite);
		}
		node->flash();
	}
};
