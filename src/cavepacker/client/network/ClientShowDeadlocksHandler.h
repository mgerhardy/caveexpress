#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/shared/network/messages/ShowDeadlocksMessage.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"
#include "ui/UI.h"

namespace cavepacker {

class ClientShowDeadlocksHandler: public IClientProtocolHandler {
protected:
	ClientMap& _map;
public:
	ClientShowDeadlocksHandler (ClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const ShowDeadlocksMessage& msg = static_cast<const ShowDeadlocksMessage&>(message);
		const std::vector<int>& indices = msg.getDeadlockIndices();
		const int width = msg.getWidth();
		for (int index : indices) {
			const int col = index % width;
			const int row = index / width;
			Log::info(LOG_CLIENT, "Deadlock at %i:%i", col, row);
		}
	}
};

}
