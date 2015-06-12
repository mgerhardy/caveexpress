#include "tests/AbstractProtocolTest.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"

namespace cavepacker {

class ProtocolTest: public AbstractProtocolTest {
};

TEST_F(ProtocolTest, testProtocols)
{
	testSharedMessages();
	testMessage("AutoSolveStartedMessage", AutoSolveStartedMessage());
	testMessage("AutoSolveAbortedMessage", AutoSolveAbortedMessage());
	testMessage("UndoMessage", UndoMessage());
}

}
