#pragma once

#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/network/messages/UpdatePackageCountMessage.h"

class UpdatePackageCountHandler: public IClientProtocolHandler {
public:
	void execute (const IProtocolMessage& message) override
	{
		// const UpdatePackageCountMessage *msg = static_cast<const UpdatePackageCountMessage*>(&message);
	}
};
