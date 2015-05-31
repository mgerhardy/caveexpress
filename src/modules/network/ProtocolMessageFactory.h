#pragma once

#include "network/IProtocolMessage.h"
#include "common/ByteStream.h"

#include "network/messages/PingMessage.h"
#include "network/messages/LoadMapMessage.h"
#include "network/messages/ClientInitMessage.h"
#include "network/messages/ErrorMessage.h"
#include "network/messages/ChangeAnimationMessage.h"
#include "network/messages/MapRestartMessage.h"
#include "network/messages/TextMessage.h"
#include "network/messages/PauseMessage.h"
#include "network/messages/AddEntityMessage.h"
#include "network/messages/UpdateEntityMessage.h"
#include "network/messages/MapSettingsMessage.h"
#include "network/messages/InitDoneMessage.h"
#include "network/messages/RemoveEntityMessage.h"
#include "network/messages/SpawnInfoMessage.h"
#include "network/messages/RumbleMessage.h"
#include "network/messages/BackToMainMessage.h"
#include "network/messages/FinishedMapMessage.h"
#include "network/messages/StopMovementMessage.h"
#include "network/messages/MovementMessage.h"
#include "network/messages/FingerMovementMessage.h"
#include "network/messages/FailedMapMessage.h"
#include "network/messages/UpdateParticleMessage.h"
#include "network/messages/SoundMessage.h"
#include "network/messages/UpdateHitpointsMessage.h"
#include "network/messages/UpdateLivesMessage.h"
#include "network/messages/TimeRemainingMessage.h"
#include "network/messages/UpdatePointsMessage.h"
#include "network/messages/UpdatePackageCountMessage.h"

class IProtocolMessage;
class ByteStream;

class ProtocolMessageFactory {
private:
	ProtocolMessageFactory ();

public:
	bool isNewMessageAvailable(const ByteStream& in) const {
		const int16_t size = in.peekShort();
		if (size == -1) {
			// not enough data yet, wait a little bit more
			return false;
		}
		const int streamSize = static_cast<int>(in.getSize() - sizeof(int16_t));
		if (size > streamSize) {
			// not enough data yet, wait a little bit more
			return false;
		}
		return true;
	}

protected:
	virtual IProtocolMessage *getForProtocolId (ByteStream& stream, const protocolId& type) const {
		switch (type) {
		case protocol::PROTO_PING:
			return new PingMessage(stream);
		case protocol::PROTO_LOADMAP:
			return new LoadMapMessage(stream);
		case protocol::PROTO_SPAWN:
			return new SpawnMessage();
		case protocol::PROTO_PLAYERLIST:
			return new PlayerListMessage(stream);
		case protocol::PROTO_STARTMAP:
			return new StartMapMessage();
		case protocol::PROTO_INITWAITING:
			return new InitWaitingMapMessage();
		case protocol::PROTO_DISCONNECT:
			return new DisconnectMessage();
		case protocol::PROTO_STOPMOVEMENT:
			return new StopMovementMessage(stream);
		case protocol::PROTO_MOVEMENT:
			return new MovementMessage(stream);
		case protocol::PROTO_FINGERMOVEMENT:
			return new FingerMovementMessage(stream);
		case protocol::PROTO_STOPFINGERMOVEMENT:
			return new StopFingerMovementMessage();
		case protocol::PROTO_CLIENTINIT:
			return new ClientInitMessage(stream);
		case protocol::PROTO_ERROR:
			return new ErrorMessage(stream);
		case protocol::PROTO_CHANGEANIMATION:
			return new ChangeAnimationMessage(stream);
		case protocol::PROTO_MAPRESTART:
			return new MapRestartMessage(stream);
		case protocol::PROTO_PAUSE:
			return new PauseMessage(stream);
		case protocol::PROTO_UPDATEENTITY:
			return new UpdateEntityMessage(stream);
		case protocol::PROTO_ADDENTITY:
			return new AddEntityMessage(stream);
		case protocol::PROTO_MAPSETTINGS:
			return new MapSettingsMessage(stream);
		case protocol::PROTO_CLOSEMAP:
			return new CloseMapMessage();
		case protocol::PROTO_FINISHEDMAP:
			return new FinishedMapMessage(stream);
		case protocol::PROTO_FAILEDMAP:
			return new FailedMapMessage(stream);
		case protocol::PROTO_SPAWNINFO:
			return new SpawnInfoMessage(stream);
		case protocol::PROTO_INITDONE:
			return new InitDoneMessage(stream);
		case protocol::PROTO_REMOVEENTITY:
			return new RemoveEntityMessage(stream);
		case protocol::PROTO_SOUND:
			return new SoundMessage(stream);
		case protocol::PROTO_RUMBLE:
			return new RumbleMessage(stream);
		case protocol::PROTO_BACKTOMAIN:
			return new BackToMainMessage(stream);
		case protocol::PROTO_MESSAGE:
			return new TextMessage(stream);
		case protocol::PROTO_UPDATEPARTICLE:
			return new UpdateParticleMessage(stream);
		case protocol::PROTO_UPDATEHITPOINTS:
			return new UpdateHitpointsMessage(stream);
		case protocol::PROTO_UPDATELIVES:
			return new UpdateLivesMessage(stream);
		case protocol::PROTO_UPDATEPOINTS:
			return new UpdatePointsMessage(stream);
		case protocol::PROTO_UPDATEPACKAGECOUNT:
			return new UpdatePackageCountMessage(stream);
		case protocol::PROTO_TIMEREMAINING:
			return new TimeRemainingMessage(stream);
		default:
			return nullptr;
		}
	}

public:
	static ProtocolMessageFactory& get ()
	{
		static ProtocolMessageFactory _instance;
		return _instance;
	}

	virtual ~ProtocolMessageFactory() {}

	IProtocolMessage *create (ByteStream& stream) const;
};
