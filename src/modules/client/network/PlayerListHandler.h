#pragma once

#include "network/IProtocolHandler.h"
#include "client/ClientMap.h"
#include "ui/UI.h"
#include "ui/nodes/IUINodeMap.h"

class PlayerListHandler: public ClientProtocolHandler<PlayerListMessage> {
private:
	IUINodeMap* _mapNode;
public:
	PlayerListHandler (IUINodeMap* mapNode) :
			_mapNode(mapNode)
	{
	}
	void execute (const PlayerListMessage* msg) override
	{
		const std::vector<std::string>& list = msg->getList();
		_mapNode->setPlayerList(list);
	}
};
