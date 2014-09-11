#include "tests/cavepacker/ProtocolTest.h"
#include "tests/AbstractProtocolTest.h"

class ProtocolTest: public AbstractProtocolTest {
};

TEST_F(ProtocolTest, testProtocols)
{
	testSharedMessages();
	//testMessage()
}
