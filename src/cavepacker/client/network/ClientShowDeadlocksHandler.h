#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/shared/network/messages/ShowDeadlocksMessage.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"
#include "ui/UI.h"

namespace cavepacker {

class ClientShowDeadlocksHandler: public IClientProtocolHandler {
protected:
	CavePackerClientMap& _map;
public:
	ClientShowDeadlocksHandler (CavePackerClientMap& map) :
			_map(map)
	{
	}

	void execute (const IProtocolMessage& message) override
	{
		const ShowDeadlocksMessage& msg = static_cast<const ShowDeadlocksMessage&>(message);
		const std::vector<int>& indices = msg.getDeadlockIndices();
		_map.setDeadlocks(indices);
	}
};

}
