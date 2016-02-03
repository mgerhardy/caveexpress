#pragma once

#include "network/IProtocolHandler.h"
#include "network/messages/TextMessage.h"

class TextMessageHandler: public ClientProtocolHandler<TextMessage> {
private:
	IUINodeMap *_mapNode;
public:
	TextMessageHandler (IUINodeMap* mapNode) :
			_mapNode(mapNode)
	{
	}

	void execute (const TextMessage* msg) override
	{
		_mapNode->displayText(msg->getMessage());
	}
};
