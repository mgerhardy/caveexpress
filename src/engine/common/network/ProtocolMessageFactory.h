#pragma once

#include "engine/common/network/IProtocolMessage.h"
#include "engine/common/ByteStream.h"

#include "engine/common/network/messages/PingMessage.h"
#include "engine/common/network/messages/LoadMapMessage.h"
#include "engine/common/network/messages/ClientInitMessage.h"
#include "engine/common/network/messages/ErrorMessage.h"
#include "engine/common/network/messages/ChangeAnimationMessage.h"
#include "engine/common/network/messages/MapRestartMessage.h"
#include "engine/common/network/messages/TextMessage.h"
#include "engine/common/network/messages/PauseMessage.h"
#include "engine/common/network/messages/AddEntityMessage.h"
#include "engine/common/network/messages/UpdateEntityMessage.h"
#include "engine/common/network/messages/MapSettingsMessage.h"
#include "engine/common/network/messages/InitDoneMessage.h"
#include "engine/common/network/messages/RemoveEntityMessage.h"
#include "engine/common/network/messages/SpawnInfoMessage.h"
#include "engine/common/network/messages/RumbleMessage.h"
#include "engine/common/network/messages/BackToMainMessage.h"
#include "engine/common/network/messages/FinishedMapMessage.h"
#include "engine/common/network/messages/StopMovementMessage.h"
#include "engine/common/network/messages/MovementMessage.h"
#include "engine/common/network/messages/FingerMovementMessage.h"
#include "engine/common/network/messages/FailedMapMessage.h"
#include "engine/common/network/messages/UpdateParticleMessage.h"
#include "engine/common/network/messages/SoundMessage.h"
#include "engine/common/network/messages/UpdateHitpointsMessage.h"
#include "engine/common/network/messages/UpdateLivesMessage.h"
#include "engine/common/network/messages/TimeRemainingMessage.h"
#include "engine/common/network/messages/UpdatePointsMessage.h"
#include "engine/common/network/messages/UpdatePackageCountMessage.h"

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
