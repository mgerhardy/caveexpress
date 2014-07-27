#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/UpdatePointsMessage.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/UINodePoint.h"
#include "engine/client/ui/windows/IUIMapWindow.h"

class UpdatePointsHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const UpdatePointsMessage *msg = static_cast<const UpdatePointsMessage*>(&message);
		const uint16_t points = msg->getPoints();
		UINodePoint* node = UI::get().getNode<UINodePoint>(UI_WINDOW_MAP, UINODE_POINTS);
		if (!node)
			return;
		node->addPoints(points);
		node->flash();
	}
};
