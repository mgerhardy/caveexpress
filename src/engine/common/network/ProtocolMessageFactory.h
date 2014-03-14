#pragma once

class IProtocolMessage;
class ByteStream;

class ProtocolMessageFactory {
private:
	ProtocolMessageFactory ();
public:

	static ProtocolMessageFactory& get ()
	{
		static ProtocolMessageFactory _instance;
		return _instance;
	}

	IProtocolMessage *create (ByteStream& stream);
};
