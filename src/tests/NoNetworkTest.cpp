#include "tests/NetworkTest.h"

#include "engine/common/network/NoNetwork.h"
#include "engine/common/Logger.h"
#include "engine/common/EntityType.h"
#include "engine/common/Animation.h"
#include "engine/common/EntityAlignment.h"
#include "engine/common/network/messages/AddEntityMessage.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"

class NetworkTestServerListener: public IServerCallback {
public:
	void onConnection (ClientId clientId) {
		// debugVA("client connected: %i", clientId);
	}
};

namespace {
const char *LOCALHOST = "localhost";
const int PORT = 4567;
}

static NetworkTestServerListener serverListener;

TEST(NoNetworkTest, testSendToClient)
{
	class NetworkTestListener: public IClientCallback {
	public:
		NetworkTestListener() : _count(0) {}
		int _count;
		void onData (ByteStream& data) {
			ProtocolMessageFactory& factory = ProtocolMessageFactory::get();
			++_count;
			ASSERT_TRUE(factory.isNewMessageAvailable(data));
			data.readShort();
			const ScopedPtr<IProtocolMessage> msg(factory.create(data));
			ASSERT_TRUE(msg);
		}
	};
	NetworkTestListener listener;
	NoNetwork network;
	ASSERT_TRUE(network.openServer(PORT, &serverListener)) << "Failed to open the server";
	ASSERT_TRUE(network.openClient(LOCALHOST, PORT, &listener)) << "Failed to open the client";
	const AddEntityMessage msg(1, EntityType::NONE, Animation::NONE, "spriteId", 0.0f, 0.0f, 0.0f, 0.0f, 0, ENTITY_ALIGN_MIDDLE_CENTER);
	network.update(0);
	int n = 1000;
	for (int i = 0; i < n; ++i) {
		network.sendToClients(0, msg);
	}
	network.update(0);
	network.closeClient();
	network.closeServer();
	ASSERT_EQ(n, listener._count);
}
