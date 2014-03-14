#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/FinishedMapMessage.h"
#include "engine/common/network/INetwork.h"
#include "caveexpress/client/ui/windows/UIMapFinishedWindow.h"
#include "engine/client/ui/nodes/UINodePoint.h"
#include "caveexpress/client/ui/nodes/UINodeStar.h"
#include "engine/client/ClientMap.h"

class FinishedMapHandler: public IClientProtocolHandler {
private:
	ClientMap& _clientMap;

	void showFinishedWindow (const FinishedMapMessage& msg) const
	{
		UI::get().pop(); // pop the map
		UI::get().push(UI_WINDOW_MAPFINISHED);
		UINodePoint *finishedPointsLabel = UI::get().getNode<UINodePoint>(UI_WINDOW_MAPFINISHED, UINODE_FINISHEDPOINTS);
		finishedPointsLabel->setPoints(msg.getFinishPoints());
		UINodeStar *star1 = UI::get().getNode<UINodeStar>(UI_WINDOW_MAPFINISHED, "star0");
		UINodeStar *star2 = UI::get().getNode<UINodeStar>(UI_WINDOW_MAPFINISHED, "star1");
		UINodeStar *star3 = UI::get().getNode<UINodeStar>(UI_WINDOW_MAPFINISHED, "star2");
		star1->disable();
		star2->disable();
		star3->disable();
		if (msg.getStars() >= 1)
			star1->enable(500);
		if (msg.getStars() >= 2)
			star2->enable(1000);
		if (msg.getStars() >= 3)
			star3->enable(1500);
		System.track("MapStats", String::format("%i:%i:%i:%s", msg.getTime(), msg.getStars(), msg.getFinishPoints(), msg.getMapName().c_str()));
	}
public:
	FinishedMapHandler (ClientMap& clientMap) :
			_clientMap(clientMap)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		_clientMap.resetCurrentMap();
		Commands.executeCommandLine(CMD_CL_DISCONNECT);

		const FinishedMapMessage *msg = static_cast<const FinishedMapMessage*>(&message);
		showFinishedWindow(*msg);
	}
};
