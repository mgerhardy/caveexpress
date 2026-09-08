#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdateLivesMessage.h"
#include "client/ClientMap.h"
#include "ui/UI.h"
#include "ui/windows/IUIMapWindow.h"
#include "campaign/CampaignManager.h"

class UpdateLivesHandler: public ClientProtocolHandler<UpdateLivesMessage> {
private:
	CampaignManager& _campaignManager;
	ClientMap& _map;
public:
	UpdateLivesHandler (CampaignManager& campaignManager, ClientMap& map) :
			_campaignManager(campaignManager), _map(map)
	{
	}

	void execute (const UpdateLivesMessage* msg) override
	{
		if (_map.isLocalPlayerSpectating())
			return;
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
