#pragma once

#include "network/IProtocolMessage.h"
#include "cavepacker/shared/network/ProtocolMessageTypes.h"

namespace cavepacker {

/**
 * @brief Message that is sent to the client if there is a deadlock state on the board
 *
 * This can also be requested by the client via @c RequestDeadlocksMessage
 */
class ShowDeadlocksMessage: public IProtocolMessage {
private:
	int _width;
	int _height;
	std::vector<int> _indices;
	const std::vector<int>* _indicesPtr;

public:
	explicit ShowDeadlocksMessage (int width, int height, const std::vector<int>& indices) :
			IProtocolMessage(protocol::PROTO_SHOWDEADLOCKS), _width(width), _height(height), _indicesPtr(&indices)
	{
	}

	PROTOCOL_CLASS_FACTORY(ShowDeadlocksMessage);

	explicit ShowDeadlocksMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_SHOWDEADLOCKS), _indicesPtr(nullptr)
	{
		_width = input.readInt();
		_height = input.readInt();
		const int16_t size = input.readShort();
		for (int i = 0; i < size; ++i) {
			_indices.push_back(input.readInt());
		}
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addInt(_width);
		out.addInt(_height);
		out.addShort((int16_t)_indicesPtr->size());
		for (int index : *_indicesPtr) {
			out.addInt(index);
		}
	}

	inline int32_t getWidth () const
	{
		return _width;
	}

	inline int32_t getHeight () const
	{
		return _height;
	}

	inline const std::vector<int>& getDeadlockIndices () const
	{
		return _indices;
	}
};

}
