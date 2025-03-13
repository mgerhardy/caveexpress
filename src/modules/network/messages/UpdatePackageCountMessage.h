#pragma once

#include "network/IProtocolMessage.h"

class UpdatePackageCountMessage: public IProtocolMessage {
private:
	uint8_t _transfers;
	uint8_t _transfersNeeded;

public:
	explicit UpdatePackageCountMessage (uint8_t packages, uint8_t packagesNeeded) :
			IProtocolMessage(protocol::PROTO_UPDATEPACKAGECOUNT),
			_transfers(packages), _transfersNeeded(packagesNeeded)
	{
	}

	PROTOCOL_CLASS_FACTORY(UpdatePackageCountMessage);

	explicit UpdatePackageCountMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_UPDATEPACKAGECOUNT)
	{
		_transfers = input.readByte();
		_transfersNeeded = input.readByte();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_transfers);
		out.addByte(_transfersNeeded);
	}

	inline uint8_t getPackages () const
	{
		return _transfers;
	}

	inline uint8_t getPackagesNeeded () const
	{
		return _transfersNeeded;
	}
};
