#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/LoadMapMessage.h"
#include "service/ServiceProvider.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeBar.h"
#include "client/ClientMap.h"
#include "ui/UI.h"
#include "common/Log.h"

class LoadMapHandler: public ClientProtocolHandler<LoadMapMessage> {
protected:
	ClientMap& _map;
	ServiceProvider& _serviceProvider;

public:
	LoadMapHandler (ClientMap& map, ServiceProvider& serviceProvider) :
			_map(map), _serviceProvider(serviceProvider)
	{
	}

	virtual void execute (const LoadMapMessage* msg) override
	{
		while (true) {
			UIWindow* front = UI::get().getFrontWindow();
			if (front == nullptr)
				break;
			const std::string& id = front->getId();
			if (id != UI_WINDOW_MAPFAILED && id != UI_WINDOW_MAPFINISHED && id != UI_WINDOW_GAMEOVER)
				break;
			UI::get().pop();
		}
		const SpawnMessage spawnMsg;
		if (_serviceProvider.getNetwork().sendToServer(spawnMsg) == -1) {
			Log::error(LOG_CLIENT, "could not send spawn command to server");
			return;
		}
		if (!UI::get().isOnStack(UI_WINDOW_MAP))
			UI::get().push(UI_WINDOW_MAP);

		System.track("mapload", msg->getName());
		UINodePoint* pointsNode = UI::get().getNode<UINodePoint>(UI_WINDOW_MAP, UINODE_POINTS);
		if (pointsNode)
			pointsNode->setPoints(0);
		UINodeBar* hitpointsBar = UI::get().getNode<UINodeBar>(UI_WINDOW_MAP, UINODE_HITPOINTS);
		if (hitpointsBar) {
			hitpointsBar->setBarColor(colorGreen);
			hitpointsBar->setBorderColor(colorGreen);
			hitpointsBar->setCurrent(100);
			hitpointsBar->reset();
		}
		_map.load(msg->getName(), msg->getTitle());
	}
};
