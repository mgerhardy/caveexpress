#pragma once

#include "IProtocolMessage.h"
#include "common/ByteStream.h"
#include "common/IFactoryRegistry.h"

class ProtocolMessageFactory: public IFactoryRegistry<protocolId, IProtocolMessage, ByteStream> {
private:
	ProtocolMessageFactory();

public:
	bool isNewMessageAvailable(const ByteStream& in) const;

public:
	static ProtocolMessageFactory& get() {
		static ProtocolMessageFactory _instance;
		return _instance;
	}

	IProtocolMessage *createMsg(ByteStream& stream) const;
};
