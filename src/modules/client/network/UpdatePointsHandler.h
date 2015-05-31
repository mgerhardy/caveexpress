#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdatePointsMessage.h"
#include "ui/UI.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/windows/IUIMapWindow.h"

class UpdatePointsHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		const UpdatePointsMessage *msg = static_cast<const UpdatePointsMessage*>(&message);
		const uint16_t points = msg->getPoints();
		UINodePoint* node = UI::get().getNode<UINodePoint>(UI_WINDOW_MAP, UINODE_POINTS);
		if (!node)
			return;
		node->increasePoints(points);
		node->flash();
	}
};
