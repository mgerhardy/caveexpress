#include "ProtocolTest.h"
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
	testMessage(AddCaveMessage(1, false));
	testMessage(AddRopeMessage(1, 2));
	testMessage(LightStateMessage(1, false));
	testMessage(RemoveRopeMessage(1));
	testMessage(UpdateCollectedTypeMessage(EntityType::NONE, false));
	testMessage(WaterHeightMessage(1.0f));
	testMessage(WaterImpactMessage(1.0f, 1.0f));
	testMessage(DropMessage());
}
