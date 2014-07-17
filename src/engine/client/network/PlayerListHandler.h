#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/client/ClientMap.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ui/nodes/IUINodeMap.h"

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
