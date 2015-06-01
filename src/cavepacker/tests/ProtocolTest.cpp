#include "ProtocolTest.h"
#include "tests/AbstractProtocolTest.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"

class ProtocolTest: public AbstractProtocolTest {
};

TEST_F(ProtocolTest, testProtocols)
{
	testSharedMessages();
	testMessage(AutoSolveStartedMessage());
	testMessage(AutoSolveAbortedMessage());
	testMessage(UndoMessage());
}
