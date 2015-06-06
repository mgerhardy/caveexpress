#include "tests/AbstractProtocolTest.h"
#include "caveexpress/shared/network/messages/AddCaveMessage.h"
#include "caveexpress/shared/network/messages/AddRopeMessage.h"
#include "caveexpress/shared/network/messages/LightStateMessage.h"
#include "caveexpress/shared/network/messages/RemoveRopeMessage.h"
#include "caveexpress/shared/network/messages/UpdateCollectedTypeMessage.h"
#include "caveexpress/shared/network/messages/WaterHeightMessage.h"
#include "caveexpress/shared/network/messages/WaterImpactMessage.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"

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
}
