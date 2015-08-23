#pragma once

#include "network/IProtocolMessage.h"
#include "cavepacker/shared/network/ProtocolMessageTypes.h"

namespace cavepacker {

/**
 * @brief Message that is sent to the server to let the player walk to the given location.
 *
 * The server will answer with single UpdateEntityMessage's
 */
class WalkToMessage: public IProtocolMessage {
private:
	int32_t _col;
	int32_t _row;

public:
	explicit WalkToMessage (int32_t col, int32_t row) :
			IProtocolMessage(protocol::PROTO_WALKTO), _col(col), _row(row)
	{
	}

	PROTOCOL_CLASS_FACTORY(WalkToMessage);

	explicit WalkToMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_WALKTO)
	{
		_col = input.readInt();
		_row = input.readInt();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addInt(_col);
		out.addInt(_row);
	}

	inline int32_t getCol () const
	{
		return _col;
	}

	inline int32_t getRow () const
	{
		return _row;
	}
};

}
