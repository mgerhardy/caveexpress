#pragma once

#include "network/IProtocolHandler.h"
#include "cavepacker/shared/network/messages/ShowDeadlocksMessage.h"
#include "cavepacker/client/ui/windows/UIMapWindow.h"
#include "ui/UI.h"

namespace cavepacker {

class ClientShowDeadlocksHandler: public ClientProtocolHandler<ShowDeadlocksMessage> {
protected:
	CavePackerClientMap& _map;
public:
	ClientShowDeadlocksHandler (CavePackerClientMap& map) :
			_map(map)
	{
	}

	void execute (const ShowDeadlocksMessage* msg) override
	{
		const std::vector<int>& indices = msg->getDeadlockIndices();
		_map.setDeadlocks(indices);
	}
};

}
