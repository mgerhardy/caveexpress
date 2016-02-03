#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/UpdatePointsMessage.h"
#include "ui/UI.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/windows/IUIMapWindow.h"

class UpdatePointsHandler: public ClientProtocolHandler<UpdatePointsMessage> {
public:
	void execute (const UpdatePointsMessage* msg) override
	{
		const uint16_t points = msg->getPoints();
		UINodePoint* node = UI::get().getNode<UINodePoint>(UI_WINDOW_MAP, UINODE_POINTS);
		if (!node)
			return;
		node->increasePoints(points);
		node->flash();
	}
};
