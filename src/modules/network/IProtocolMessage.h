#pragma once

#include "common/ByteStream.h"
#include "common/Compiler.h"
#include "common/Pointers.h"
#include "common/IFactoryRegistry.h"
#include <vector>
#include <string>

typedef uint8_t protocolId;

#define PROTOCOL_CLASS_FACTORY(className) \
	class Factory: public IProtocolMessageFactory { \
	private: \
		uint8_t* _memory; \
		mutable bool _initialized; \
	public: \
		Factory() : _memory(new uint8_t[sizeof(className)]), _initialized(false) {} \
		~Factory() { if (_initialized) { className* obj = reinterpret_cast<className*>(_memory); obj->~className(); } delete[] _memory; } \
		IProtocolMessage* create (const ByteStream *ctx) const { \
			_initialized = true; \
			return new (_memory) className(*const_cast<ByteStream*>(ctx)); \
		} \
	}; \
	static Factory FACTORY

#define PROTOCOL_CLASS_FACTORY_IMPL(className) \
	className::Factory className::FACTORY

#define PROTOCOL_CLASS_SIMPLE(className, id) \
	class className : public IProtocolMessage { \
		public: \
			PROTOCOL_CLASS_FACTORY(className); \
			className () : IProtocolMessage(id) {} \
			className (ByteStream& input) : IProtocolMessage(id) {} \
			void serialize (ByteStream& out) const override { out.addByte(_id); } \
	}
#define PROTOCOL_CLASS_SIMPLE_LIST(className, id) \
	class className : public IProtocolListMessage { \
		public: \
			PROTOCOL_CLASS_FACTORY(className); \
			className (const std::vector<std::string>& list) : IProtocolListMessage(id, list) {} \
			className (ByteStream& input) : IProtocolListMessage(id, input) {} \
	}
class IProtocolMessage {
protected:
	protocolId _id;

public:
	IProtocolMessage (protocolId id) :
			_id(id)
	{
	}

	virtual ~IProtocolMessage ()
	{
	}

	inline protocolId getId () const
	{
		return _id;
	}

	virtual void serialize (ByteStream& out) const = 0;
};

typedef SharedPtr<IProtocolMessage> ProtocolMessagePtr;

class IProtocolListMessage: public IProtocolMessage {
protected:
	typedef std::vector<std::string> List;
	List _list;
	const List* _listPtr;
public:
	IProtocolListMessage (protocolId id, const std::vector<std::string>& list) :
			IProtocolMessage(id), _listPtr(&list)
	{
	}

	IProtocolListMessage (protocolId id, ByteStream& input) :
			IProtocolMessage(id), _listPtr(nullptr)
	{
		const int16_t size = input.readShort();
		for (int16_t i = 0; i < size; ++i) {
			const std::string value = input.readString();
			_list.push_back(value);
		}
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_listPtr->size());
		for (List::const_iterator i = _listPtr->begin(); i != _listPtr->end(); ++i) {
			out.addString(*i);
		}
	}

	inline const std::vector<std::string>& getList () const
	{
		return _list;
	}
};

class IProtocolMessageFactory: public IFactory<IProtocolMessage, ByteStream> {
public:
	virtual ~IProtocolMessageFactory ()
	{
	}
	virtual IProtocolMessage* create (const ByteStream *ctx) const = 0;
};

namespace protocol
{
const uint8_t PROTO_UPDATEHITPOINTS = 221;
const uint8_t PROTO_UPDATELIVES = 222;
const uint8_t PROTO_TIMEREMAINING = 223;
const uint8_t PROTO_UPDATEPOINTS = 224;
const uint8_t PROTO_UPDATEPACKAGECOUNT = 225;
const uint8_t PROTO_STOPMOVEMENT = 226;
const uint8_t PROTO_MOVEMENT = 227;
const uint8_t PROTO_FINGERMOVEMENT = 228;
const uint8_t PROTO_STOPFINGERMOVEMENT = 229;
const uint8_t PROTO_FAILEDMAP = 230;
const uint8_t PROTO_UPDATEPARTICLE = 231;
const uint8_t PROTO_LOADMAP = 232;
const uint8_t PROTO_SPAWN = 233;
const uint8_t PROTO_SPAWNINFO = 234;
const uint8_t PROTO_CLIENTINIT = 235;
const uint8_t PROTO_ERROR = 236;
const uint8_t PROTO_MAPRESTART = 237;
const uint8_t PROTO_PAUSE = 238;
const uint8_t PROTO_MAPSETTINGS = 239;
const uint8_t PROTO_CLOSEMAP = 240;
const uint8_t PROTO_INITDONE = 241;
const uint8_t PROTO_SOUND = 242;
const uint8_t PROTO_RUMBLE = 243;
const uint8_t PROTO_FINISHEDMAP = 244;
const uint8_t PROTO_STARTMAP = 245;
const uint8_t PROTO_PLAYERLIST = 246;
const uint8_t PROTO_INITWAITING = 247;
const uint8_t PROTO_MESSAGE = 248;
const uint8_t PROTO_UPDATEENTITY = 249;
const uint8_t PROTO_REMOVEENTITY = 250;
const uint8_t PROTO_CHANGEANIMATION = 251;
const uint8_t PROTO_BACKTOMAIN = 252;
const uint8_t PROTO_ADDENTITY = 253;
const uint8_t PROTO_PING = 254;
const uint8_t PROTO_DISCONNECT = 255;
}

PROTOCOL_CLASS_SIMPLE(DisconnectMessage, protocol::PROTO_DISCONNECT);
// This is the message that is sent to all the connected clients in the multiplayer game
// that the game is starting now.
PROTOCOL_CLASS_SIMPLE(StartMapMessage, protocol::PROTO_STARTMAP);
PROTOCOL_CLASS_SIMPLE(CloseMapMessage, protocol::PROTO_CLOSEMAP);
// Multiplayer message that sends the current connected players to the clients
PROTOCOL_CLASS_SIMPLE_LIST(PlayerListMessage, protocol::PROTO_PLAYERLIST);
// This message is only sent in multiplayer mode and initializes the waiting dialogs until the
// server admin starts the server or the max amount of clients are connected
PROTOCOL_CLASS_SIMPLE(InitWaitingMapMessage, protocol::PROTO_INITWAITING);
PROTOCOL_CLASS_SIMPLE(SpawnMessage, protocol::PROTO_SPAWN);
PROTOCOL_CLASS_SIMPLE(StopFingerMovementMessage, protocol::PROTO_STOPFINGERMOVEMENT);
