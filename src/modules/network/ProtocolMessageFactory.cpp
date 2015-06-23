#include "ProtocolMessageFactory.h"

#include "messages/PingMessage.h"
#include "messages/LoadMapMessage.h"
#include "messages/ClientInitMessage.h"
#include "messages/ErrorMessage.h"
#include "messages/ChangeAnimationMessage.h"
#include "messages/MapRestartMessage.h"
#include "messages/TextMessage.h"
#include "messages/PauseMessage.h"
#include "messages/AddEntityMessage.h"
#include "messages/UpdateEntityMessage.h"
#include "messages/MapSettingsMessage.h"
#include "messages/InitDoneMessage.h"
#include "messages/RemoveEntityMessage.h"
#include "messages/SpawnInfoMessage.h"
#include "messages/RumbleMessage.h"
#include "messages/BackToMainMessage.h"
#include "messages/FinishedMapMessage.h"
#include "messages/StopMovementMessage.h"
#include "messages/MovementMessage.h"
#include "messages/FingerMovementMessage.h"
#include "messages/FailedMapMessage.h"
#include "messages/UpdateParticleMessage.h"
#include "messages/SoundMessage.h"
#include "messages/UpdateHitpointsMessage.h"
#include "messages/UpdateLivesMessage.h"
#include "messages/TimeRemainingMessage.h"
#include "messages/UpdatePointsMessage.h"
#include "messages/UpdatePackageCountMessage.h"

PROTOCOL_CLASS_FACTORY_IMPL(PingMessage);
PROTOCOL_CLASS_FACTORY_IMPL(LoadMapMessage);
PROTOCOL_CLASS_FACTORY_IMPL(SpawnMessage);
PROTOCOL_CLASS_FACTORY_IMPL(PlayerListMessage);
PROTOCOL_CLASS_FACTORY_IMPL(StartMapMessage);
PROTOCOL_CLASS_FACTORY_IMPL(InitWaitingMapMessage);
PROTOCOL_CLASS_FACTORY_IMPL(DisconnectMessage);
PROTOCOL_CLASS_FACTORY_IMPL(StopMovementMessage);
PROTOCOL_CLASS_FACTORY_IMPL(MovementMessage);
PROTOCOL_CLASS_FACTORY_IMPL(FingerMovementMessage);
PROTOCOL_CLASS_FACTORY_IMPL(StopFingerMovementMessage);
PROTOCOL_CLASS_FACTORY_IMPL(ClientInitMessage);
PROTOCOL_CLASS_FACTORY_IMPL(ErrorMessage);
PROTOCOL_CLASS_FACTORY_IMPL(ChangeAnimationMessage);
PROTOCOL_CLASS_FACTORY_IMPL(MapRestartMessage);
PROTOCOL_CLASS_FACTORY_IMPL(PauseMessage);
PROTOCOL_CLASS_FACTORY_IMPL(UpdateEntityMessage);
PROTOCOL_CLASS_FACTORY_IMPL(AddEntityMessage);
PROTOCOL_CLASS_FACTORY_IMPL(MapSettingsMessage);
PROTOCOL_CLASS_FACTORY_IMPL(CloseMapMessage);
PROTOCOL_CLASS_FACTORY_IMPL(FinishedMapMessage);
PROTOCOL_CLASS_FACTORY_IMPL(FailedMapMessage);
PROTOCOL_CLASS_FACTORY_IMPL(SpawnInfoMessage);
PROTOCOL_CLASS_FACTORY_IMPL(InitDoneMessage);
PROTOCOL_CLASS_FACTORY_IMPL(RemoveEntityMessage);
PROTOCOL_CLASS_FACTORY_IMPL(SoundMessage);
PROTOCOL_CLASS_FACTORY_IMPL(RumbleMessage);
PROTOCOL_CLASS_FACTORY_IMPL(BackToMainMessage);
PROTOCOL_CLASS_FACTORY_IMPL(TextMessage);
PROTOCOL_CLASS_FACTORY_IMPL(UpdateParticleMessage);
PROTOCOL_CLASS_FACTORY_IMPL(UpdateHitpointsMessage);
PROTOCOL_CLASS_FACTORY_IMPL(UpdateLivesMessage);
PROTOCOL_CLASS_FACTORY_IMPL(UpdatePointsMessage);
PROTOCOL_CLASS_FACTORY_IMPL(UpdatePackageCountMessage);
PROTOCOL_CLASS_FACTORY_IMPL(TimeRemainingMessage);

ProtocolMessageFactory::ProtocolMessageFactory() {
	registerFactory(protocol::PROTO_PING, PingMessage::FACTORY);
	registerFactory(protocol::PROTO_LOADMAP, LoadMapMessage::FACTORY);
	registerFactory(protocol::PROTO_SPAWN, SpawnMessage::FACTORY);
	registerFactory(protocol::PROTO_PLAYERLIST, PlayerListMessage::FACTORY);
	registerFactory(protocol::PROTO_STARTMAP, StartMapMessage::FACTORY);
	registerFactory(protocol::PROTO_INITWAITING, InitWaitingMapMessage::FACTORY);
	registerFactory(protocol::PROTO_DISCONNECT, DisconnectMessage::FACTORY);
	registerFactory(protocol::PROTO_STOPMOVEMENT, StopMovementMessage::FACTORY);
	registerFactory(protocol::PROTO_MOVEMENT, MovementMessage::FACTORY);
	registerFactory(protocol::PROTO_FINGERMOVEMENT, FingerMovementMessage::FACTORY);
	registerFactory(protocol::PROTO_STOPFINGERMOVEMENT, StopFingerMovementMessage::FACTORY);
	registerFactory(protocol::PROTO_CLIENTINIT, ClientInitMessage::FACTORY);
	registerFactory(protocol::PROTO_ERROR, ErrorMessage::FACTORY);
	registerFactory(protocol::PROTO_CHANGEANIMATION, ChangeAnimationMessage::FACTORY);
	registerFactory(protocol::PROTO_MAPRESTART, MapRestartMessage::FACTORY);
	registerFactory(protocol::PROTO_PAUSE, PauseMessage::FACTORY);
	registerFactory(protocol::PROTO_UPDATEENTITY, UpdateEntityMessage::FACTORY);
	registerFactory(protocol::PROTO_ADDENTITY, AddEntityMessage::FACTORY);
	registerFactory(protocol::PROTO_MAPSETTINGS, MapSettingsMessage::FACTORY);
	registerFactory(protocol::PROTO_CLOSEMAP, CloseMapMessage::FACTORY);
	registerFactory(protocol::PROTO_FINISHEDMAP, FinishedMapMessage::FACTORY);
	registerFactory(protocol::PROTO_FAILEDMAP, FailedMapMessage::FACTORY);
	registerFactory(protocol::PROTO_SPAWNINFO, SpawnInfoMessage::FACTORY);
	registerFactory(protocol::PROTO_INITDONE, InitDoneMessage::FACTORY);
	registerFactory(protocol::PROTO_REMOVEENTITY, RemoveEntityMessage::FACTORY);
	registerFactory(protocol::PROTO_SOUND, SoundMessage::FACTORY);
	registerFactory(protocol::PROTO_RUMBLE, RumbleMessage::FACTORY);
	registerFactory(protocol::PROTO_BACKTOMAIN, BackToMainMessage::FACTORY);
	registerFactory(protocol::PROTO_MESSAGE, TextMessage::FACTORY);
	registerFactory(protocol::PROTO_UPDATEPARTICLE, UpdateParticleMessage::FACTORY);
	registerFactory(protocol::PROTO_UPDATEHITPOINTS, UpdateHitpointsMessage::FACTORY);
	registerFactory(protocol::PROTO_UPDATELIVES, UpdateLivesMessage::FACTORY);
	registerFactory(protocol::PROTO_UPDATEPOINTS, UpdatePointsMessage::FACTORY);
	registerFactory(protocol::PROTO_UPDATEPACKAGECOUNT, UpdatePackageCountMessage::FACTORY);
	registerFactory(protocol::PROTO_TIMEREMAINING, TimeRemainingMessage::FACTORY);
}

bool ProtocolMessageFactory::isNewMessageAvailable(const ByteStream& in) const {
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

ProtocolMessageFactory::~ProtocolMessageFactory() {
}

IProtocolMessage *ProtocolMessageFactory::createMsg(ByteStream& stream) const {
	const protocolId type = stream.readByte();
	//Log::debug(LOG_GENERAL, String::format("msg type => %i", (int)type));
	IProtocolMessage *msg = create(type, &stream);
	if (msg != nullptr)
		return msg;
	stream.addByte(type, true);
	Log::error2(LOG_NET, "unknown module type given: %i", type);
	return nullptr;
}
