#pragma once

#include "network/IProtocolHandler.h"
#include "client/ClientMap.h"
#include "client/ui/UI.h"
#include "client/ui/nodes/IUINodeMap.h"

class PlayerListHandler: public IClientProtocolHandler {
private:
	IUINodeMap* _mapNode;
public:
	PlayerListHandler (IUINodeMap* mapNode) :
			_mapNode(mapNode)
	{
	}
	void execute (const IProtocolMessage& message) override
	{
		const PlayerListMessage *msg = static_cast<const PlayerListMessage*>(&message);
		const std::vector<std::string>& list = msg->getList();
		_mapNode->setPlayerList(list);
	}
};
