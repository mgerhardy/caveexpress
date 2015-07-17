#include "tests/AbstractProtocolTest.h"
#include "miniracer/shared/network/messages/ProtocolMessages.h"

namespace miniracer {

class ProtocolTest: public AbstractProtocolTest {
};

TEST_F(ProtocolTest, testProtocols)
{
	testSharedMessages();
}

}
