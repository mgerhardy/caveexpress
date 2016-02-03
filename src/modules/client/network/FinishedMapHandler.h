#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/FinishedMapMessage.h"
#include "network/INetwork.h"
#include "common/Commands.h"
#include "ui/UI.h"
#include "ui/windows/UIMapFinishedWindow.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeStar.h"
#include "client/ClientMap.h"

class FinishedMapHandler: public ClientProtocolHandler<FinishedMapMessage> {
private:
	ClientMap& _clientMap;

	void showFinishedWindow (const FinishedMapMessage& msg) const
	{
		UI::get().pop(); // pop the map
		UI::get().push(UI_WINDOW_MAPFINISHED);
		UINodePoint *finishedPointsLabel = UI::get().getNode<UINodePoint>(UI_WINDOW_MAPFINISHED, UINODE_FINISHEDPOINTS);
		if (finishedPointsLabel)
			finishedPointsLabel->setPoints(msg.getFinishPoints());
		UINodeStar *star1 = UI::get().getNode<UINodeStar>(UI_WINDOW_MAPFINISHED, "star0");
		UINodeStar *star2 = UI::get().getNode<UINodeStar>(UI_WINDOW_MAPFINISHED, "star1");
		UINodeStar *star3 = UI::get().getNode<UINodeStar>(UI_WINDOW_MAPFINISHED, "star2");
		if (star1)
			star1->disable();
		if (star2)
			star2->disable();
		if (star3)
			star3->disable();
		if (msg.getStars() >= 1 && star1)
			star1->enable(500);
		if (msg.getStars() >= 2 && star2)
			star2->enable(1000);
		if (msg.getStars() >= 3 && star3)
			star3->enable(1500);
		System.track("mapstate", string::format("%i:%i:%i:%s", msg.getTime(), msg.getStars(), msg.getFinishPoints(), msg.getMapName().c_str()));
	}
public:
	FinishedMapHandler (ClientMap& clientMap) :
			_clientMap(clientMap)
	{
	}

	void execute (const FinishedMapMessage* msg) override
	{
		_clientMap.close();
		Commands.executeCommandLine(CMD_CL_DISCONNECT);
		showFinishedWindow(*msg);
	}
};
