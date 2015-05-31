#pragma once

#include "common/network/IProtocolHandler.h"
#include "client/network/LoadMapHandler.h"
#include "client/ui/windows/IUIMapWindow.h"
#include "client/ui/nodes/IUINodeMap.h"
#include "client/ui/nodes/UINodeSprite.h"
#include "client/ui/nodes/UINodePoint.h"
#include "client/ui/windows/UIWindow.h"
#include "client/ClientMap.h"
#include "client/ui/UI.h"

class HudLoadMapHandler: public LoadMapHandler {
public:
	HudLoadMapHandler (ClientMap& map, ServiceProvider& serviceProvider) :
			LoadMapHandler(map, serviceProvider)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		LoadMapHandler::execute(message);
		const LoadMapMessage *msg = static_cast<const LoadMapMessage*>(&message);
		UINodeSprite* collectedNode = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_COLLECTED);
		if (collectedNode)
			collectedNode->clearSprites();
		UINodePoint* pointsNode = UI::get().getNode<UINodePoint>(UI_WINDOW_MAP, UINODE_POINTS);
		if (pointsNode)
			pointsNode->setPoints(0);
		IUINodeMap* mapNode = UI::get().getNode<IUINodeMap>(UI_WINDOW_MAP, UINODE_MAP);
		if (mapNode)
			mapNode->setTitle(msg->getTitle());
	}
};
