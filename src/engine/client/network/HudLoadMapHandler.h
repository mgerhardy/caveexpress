#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/client/network/LoadMapHandler.h"
#include "engine/client/ui/windows/IUIMapWindow.h"
#include "engine/client/ui/nodes/IUINodeMap.h"
#include "engine/client/ui/nodes/UINodeSprite.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ClientMap.h"
#include "engine/client/ui/UI.h"

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
		collectedNode->clearSprites();
		UINodeLabel* pointsNode = UI::get().getNode<UINodeLabel>(UI_WINDOW_MAP, UINODE_POINTS);
		pointsNode->setLabel("0");
		IUINodeMap* mapNode = UI::get().getNode<IUINodeMap>(UI_WINDOW_MAP, UINODE_MAP);
		mapNode->setTitle(msg->getTitle());
	}
};
