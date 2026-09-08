#include "tests/AbstractProtocolTest.h"
#include "caveexpress/shared/network/messages/AddCaveMessage.h"
#include "caveexpress/shared/network/messages/AddRopeMessage.h"
#include "caveexpress/shared/network/messages/LightStateMessage.h"
#include "caveexpress/shared/network/messages/GateStateMessage.h"
#include "caveexpress/shared/network/messages/RemoveRopeMessage.h"
#include "caveexpress/shared/network/messages/UpdateCollectedTypeMessage.h"
#include "caveexpress/shared/network/messages/WaterHeightMessage.h"
#include "caveexpress/shared/network/messages/WaterImpactMessage.h"
#include "caveexpress/shared/network/messages/PlayerHudMessage.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "network/messages/PingMessage.h"
#include "network/IProtocolMessage.h"
#include "network/ProtocolMessageFactory.h"

namespace caveexpress {

class ProtocolTest: public AbstractProtocolTest {
};

TEST_F(ProtocolTest, testProtocols)
{
	testSharedMessages();
	testMessage("AddCaveMessage", AddCaveMessage(1, 1, false));
	testMessage("AddRopeMessage", AddRopeMessage(1, 2));
	testMessage("LightStateMessage", LightStateMessage(1, false));
	testMessage("RemoveRopeMessage", RemoveRopeMessage(1));
	testMessage("UpdateCollectedTypeMessage", UpdateCollectedTypeMessage(EntityType::NONE, false));
	testMessage("WaterHeightMessage", WaterHeightMessage(1.0f));
	testMessage("WaterImpactMessage", WaterImpactMessage(1.0f, 1.0f));
	testMessage("DropMessage", DropMessage());
	testMessage("GateStateMessage", GateStateMessage(9, 128));
	testMessage("PlayerHudMessage", PlayerHudMessage(7, 80, 3, 2, 0));
}

TEST_F(ProtocolTest, testPingMessageOldServersOmitInGameFlag)
{
	ByteStream s;
	s.addByte(::protocol::PROTO_PING);
	s.addString("name");
	s.addString("mapName");
	s.addInt(42);
	s.addByte(1);
	s.addByte(4);
	IProtocolMessage* m = ProtocolMessageFactory::get().createMsg(s);
	ASSERT_NE(nullptr, m);
	EXPECT_TRUE(s.empty());
	const PingMessage* p = static_cast<const PingMessage*>(m);
	EXPECT_FALSE(p->isInGame());
	EXPECT_EQ(1, p->getPlayerCount());
	EXPECT_EQ(4, p->getMaxPlayerCount());
}

}
