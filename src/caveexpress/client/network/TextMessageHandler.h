#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/TextMessage.h"

class TextMessageHandler: public IClientProtocolHandler {
private:
	UINodeMap *_mapNode;
public:
	TextMessageHandler (UINodeMap* mapNode) :
			_mapNode(mapNode)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const TextMessage *msg = static_cast<const TextMessage*>(&message);
		_mapNode->displayText(msg->getMessage());
	}
};
