#include "TestShared.h"
#include "common/Config.h"

#ifndef NONETWORK

#include "common/IEventObserver.h"
#include "network/Network.h"
#include "common/Log.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"

class NetworkTestListener: public IClientCallback {
public:
	void onData (ByteStream& data){
		Log::debug(LOG_NETWORK, "recv: %i", (int)data.getSize());
	}
};

class NetworkTestServerListener: public IServerCallback {
public:
	void onConnection (ClientId clientId){
		// debugVA("client connected: %i", clientId);
	}
};

namespace {
const uint8_t BYTE_ADD = 1;
const int16_t SHORT_ADD = 64;
const int32_t INT_ADD = 128;

const char *LOCALHOST = "localhost";
const int PORT = 4567;
}

TEST(NetworkTest, testOpenServer)
{
	NetworkTestServerListener serverListener;
	Network network;
	ASSERT_TRUE(network.openServer(PORT, &serverListener)) << network.getError();
	network.closeServer();
}

TEST(NetworkTest, testOpenClient)
{
	NetworkTestListener listener;
	NetworkTestServerListener serverListener;
	Network network;
	ASSERT_TRUE(network.openServer(PORT, &serverListener)) << network.getError();
	ASSERT_TRUE(network.openClient(LOCALHOST, PORT, &listener)) << network.getError();
	network.closeClient();
	network.closeServer();
}

TEST(NetworkTest, testUpdate)
{
	NetworkTestListener listener;
	NetworkTestServerListener serverListener;
	Network network;
	ASSERT_TRUE(network.openServer(PORT, &serverListener)) << network.getError();
	ASSERT_TRUE(network.openClient(LOCALHOST, PORT, &listener)) << network.getError();
	network.update(0);
	network.closeClient();
	network.closeServer();
}

TEST(NetworkTest, testSendToClient)
{
	NetworkTestListener listener;
	NetworkTestServerListener serverListener;
	Network network;
	ASSERT_TRUE(network.openServer(PORT, &serverListener)) << network.getError();
	ASSERT_TRUE(network.openClient(LOCALHOST, PORT, &listener)) << network.getError();
	const DisconnectMessage msg;
	network.update(0);
	network.sendToClients(0, msg);
	network.update(0);
	network.closeClient();
	network.closeServer();
}

TEST(NetworkTest, testSendStringList)
{
	NetworkTestServerListener serverListener;
	std::vector<std::string> names;
	names.push_back("Test1");
	names.push_back("Test2");
	PlayerListMessage msgNames(names);
	Network network;
	ASSERT_TRUE(network.openServer(PORT, &serverListener)) << network.getError();
	class NetworkNameListTestListener: public IClientCallback {
	public:
		NetworkNameListTestListener() :
				count(0) {
		}
		int count;
		void onData(ByteStream &data) override
		{
			const int size = data.readShort();
			ASSERT_TRUE(size > 0);
			ASSERT_EQ(protocol::PROTO_PLAYERLIST, data.readByte());
			PlayerListMessage msg(data);
			ASSERT_EQ("Test1", msg.getList()[0]);
			ASSERT_EQ("Test2", msg.getList()[1]);
			++count;
		}
	};
	NetworkNameListTestListener nameListener;
	ASSERT_TRUE(network.openClient(LOCALHOST, PORT, &nameListener)) << network.getError();
	network.update(0);
	network.sendToAllClients(msgNames);
	network.update(5000);
	network.sendToAllClients(msgNames);
	network.update(5000);
	network.sendToAllClients(msgNames);
	network.update(5000);
	ASSERT_EQ(3, nameListener.count);
	network.closeClient();
	network.closeServer();
}

TEST(NetworkTest, testSendToServer)
{
	NetworkTestListener listener;
	NetworkTestServerListener serverListener;
	Network network;
	ASSERT_TRUE(network.openServer(PORT, &serverListener)) << network.getError();
	ASSERT_TRUE(network.openClient(LOCALHOST, PORT, &listener)) << network.getError();
	network.update(0);
	const DisconnectMessage msg;
	const int expectedSize = sizeof(uint16_t) + sizeof(protocolId);
	ASSERT_EQ(expectedSize, network.sendToServer(msg));
	ASSERT_EQ(1, network.sendToClients(0, msg));
	network.update(0);
	network.closeClient();
	network.closeServer();
}

#endif
