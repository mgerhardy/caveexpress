#pragma once

#include "network/IProtocolMessage.h"

class UpdateTransferCountMessage: public IProtocolMessage {
private:
	uint8_t _transfers;
	uint8_t _transfersNeeded;

public:
	explicit UpdateTransferCountMessage (uint8_t transfers, uint8_t transfersNeeded) :
			IProtocolMessage(protocol::PROTO_UPDATETRANSFERCOUNT),
			_transfers(transfers), _transfersNeeded(transfersNeeded)
	{
	}

	PROTOCOL_CLASS_FACTORY(UpdateTransferCountMessage);

	explicit UpdateTransferCountMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_UPDATETRANSFERCOUNT)
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

	inline uint8_t getTransfers () const
	{
		return _transfers;
	}

	inline uint8_t getTransfersNeeded () const
	{
		return _transfersNeeded;
	}
};
