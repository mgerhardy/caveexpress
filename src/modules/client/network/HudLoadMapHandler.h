#pragma once

#include "network/IProtocolHandler.h"
#include "client/network/LoadMapHandler.h"
#include "ui/windows/IUIMapWindow.h"
#include "ui/nodes/IUINodeMap.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/windows/UIWindow.h"
#include "client/ClientMap.h"
#include "ui/UI.h"

class HudLoadMapHandler: public LoadMapHandler {
public:
	HudLoadMapHandler (ClientMap& map, ServiceProvider& serviceProvider) :
			LoadMapHandler(map, serviceProvider)
	{
	}

	void execute (const LoadMapMessage* msg) override
	{
		LoadMapHandler::execute(msg);
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
