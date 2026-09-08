#include "caveexpress/shared/network/ProtocolMessageFactories.h"
#include "caveexpress/shared/network/messages/AddCaveMessage.h"
#include "caveexpress/shared/network/messages/AddRopeMessage.h"
#include "caveexpress/shared/network/messages/AnnounceTargetCaveMessage.h"
#include "caveexpress/shared/network/messages/GateStateMessage.h"
#include "caveexpress/shared/network/messages/LightStateMessage.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/shared/network/messages/RemoveRopeMessage.h"
#include "caveexpress/shared/network/messages/TargetCaveMessage.h"
#include "caveexpress/shared/network/messages/UpdateCollectedTypeMessage.h"
#include "caveexpress/shared/network/messages/WaterHeightMessage.h"
#include "caveexpress/shared/network/messages/WaterImpactMessage.h"
#include "caveexpress/shared/network/messages/PlayerHudMessage.h"
#include "network/ProtocolMessageFactory.h"

PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::DropMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::RemoveRopeMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::AddRopeMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::LightStateMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::AddCaveMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::UpdateCollectedTypeMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::WaterHeightMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::WaterImpactMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::TargetCaveMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::AnnounceTargetCaveMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::GateStateMessage);
PROTOCOL_CLASS_FACTORY_IMPL(caveexpress::PlayerHudMessage);

namespace caveexpress {

void registerCaveExpressProtocolMessages ()
{
	ProtocolMessageFactory& f = ProtocolMessageFactory::get();
	f.registerFactory(protocol::PROTO_DROP, DropMessage::FACTORY);
	f.registerFactory(protocol::PROTO_REMOVEROPE, RemoveRopeMessage::FACTORY);
	f.registerFactory(protocol::PROTO_WATERHEIGHT, WaterHeightMessage::FACTORY);
	f.registerFactory(protocol::PROTO_WATERIMPACT, WaterImpactMessage::FACTORY);
	f.registerFactory(protocol::PROTO_ADDCAVE, AddCaveMessage::FACTORY);
	f.registerFactory(protocol::PROTO_LIGHTSTATE, LightStateMessage::FACTORY);
	f.registerFactory(protocol::PROTO_GATESTATE, GateStateMessage::FACTORY);
	f.registerFactory(protocol::PROTO_TARGETCAVE, TargetCaveMessage::FACTORY);
	f.registerFactory(protocol::PROTO_ANNOUNCETARGETCAVE, AnnounceTargetCaveMessage::FACTORY);
	f.registerFactory(protocol::PROTO_UPDATECOLLECTEDTYPE, UpdateCollectedTypeMessage::FACTORY);
	f.registerFactory(protocol::PROTO_ADDROPE, AddRopeMessage::FACTORY);
	f.registerFactory(protocol::PROTO_PLAYERHUD, PlayerHudMessage::FACTORY);
}

}
