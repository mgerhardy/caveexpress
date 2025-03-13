#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdateTransferCountMessage.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/windows/IUIMapWindow.h"
#include <string>

namespace caveexpress {

class UpdateTransferCountHandler: public ClientProtocolHandler<UpdateTransferCountMessage> {
public:
	void execute (const UpdateTransferCountMessage* msg) override
	{
		const uint8_t transfers = msg->getTransfers();
		const uint8_t transfersNeeded = msg->getTransfersNeeded();
		
		UINodeLabel *npcLeft = UI::get().getNode<UINodeLabel>(UI_WINDOW_MAP, UINODE_TRANSFERS_LEFT);
		npcLeft->setLabel(string::format("%d/%d", transfers, transfersNeeded));
	}
};

}
