#pragma once

#include "network/IProtocolMessage.h"
#include "common/ErrorTypes.h"
#include <string>

class ErrorMessage: public IProtocolMessage {
private:
	ErrorTypes _errorType;
	uint16_t _errorId;
public:
	ErrorMessage (ErrorTypes errorType, short errorId) :
			IProtocolMessage(protocol::PROTO_ERROR), _errorType(errorType), _errorId(errorId)
	{
	}

	ErrorMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_ERROR)
	{
		_errorType = static_cast<ErrorTypes>(input.readByte());
		_errorId = input.readShort();
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addByte(_errorType);
		out.addShort(_errorId);
	}

	inline ErrorTypes getErrorType () const
	{
		return _errorType;
	}

	inline uint16_t getErrorData () const
	{
		return _errorId;
	}
};
