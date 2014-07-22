#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/FinishedMapMessage.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/Commands.h"
#include "engine/client/ui/UI.h"
#include "engine/client/ClientMap.h"

class FinishedMapHandler: public IClientProtocolHandler {
private:
	ClientMap& _clientMap;
public:
	FinishedMapHandler (ClientMap& clientMap) :
			_clientMap(clientMap)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		_clientMap.resetCurrentMap();
		Commands.executeCommandLine(CMD_CL_DISCONNECT);
	}
};
