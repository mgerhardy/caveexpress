#pragma once

#ifndef NONETWORK

#include <memory>
#include "INetwork.h"
#include <SDL_net.h>
#include <vector>

#define NET_USE_UDP

class Network : public INetwork {
private:
	class OOB {
	private:
		UDPsocket socket;
		IClientCallback *callback;
		UDPpacket *p;
		Network *network;
	public:
		OOB (UDPsocket socket_, IClientCallback *callback_, Network *network_) :
				socket(socket_), callback(callback_), network(network_)
		{
			p = SDLNet_AllocPacket(512);
		}

		inline int send (UDPpacket *packet) const
		{
			return SDLNet_UDP_Send(socket, -1, packet);
		}

		inline int recv ()
		{
			return SDLNet_UDP_Recv(socket, p);
		}

		inline int getPacketLength () const
		{
			return p->len;
		}

		void append (ByteStream& s) const
		{
			s.append(p->data, p->len);
		}

		void onOOBData (ByteStream &stream)
		{
			if (callback == nullptr) {
				return;
			}

			const IProtocolMessage* msg(ProtocolMessageFactory::get().createMsg(stream));
			if (!msg)
				return;

			const std::string host = network->toString(p->address);
			callback->onOOBData(host, msg);
		}

		virtual ~OOB ()
		{
			SDLNet_UDP_Close(socket);
			SDLNet_FreePacket(p);
		}
	};

	class Client {
	public:
		Client (TCPsocket _socket, ClientId _num) :
				socket(_socket), num(_num), disconnect(false)
		{
		}

		~Client ()
		{
			SDLNet_TCP_Close(socket);
			socket = nullptr;
		}

		ByteStream buf;
		TCPsocket socket;
		ClientId num;
		bool disconnect;
	};
	typedef std::shared_ptr<Client> ClientPtr;
	TCPsocket _serverSocket;
	TCPsocket _clientSocket;
#ifdef NET_USE_UDP
	UDPsocket _serverDatagramSocket;
	UDPpacket *_serverDatagramPacket;
#endif

	SDLNet_SocketSet _socketSet;
	typedef std::vector<ClientPtr> ClientSockets;
	ClientSockets _clients;

#ifdef NET_USE_UDP
	typedef std::vector<OOB*> OOBSockets;
	OOBSockets _oobSockets;
#endif

	// counter for unique client ids
	ClientId _clientId;

	int _bytesIn;
	int _bytesOut;

	uint32_t _time;

	bool _closing;

	int send (TCPsocket socket, const ByteStream &buffer);
	int recv (TCPsocket socket, ByteStream &buffer);

	void handleDisconnectedClients ();

	int sendToClients (int clientMask, const ByteStream& buffer);
	int sendToServer (const ByteStream& buffer);
	bool sendUDP (UDPsocket sock, const IPaddress &address, const IProtocolMessage& msg);

	// opens an out-of-band print connection
	OOB* openOOB (IClientCallback* oobCallback = nullptr, int16_t port = 0);
	// send a message to the local networks broadcast address
	bool broadcast (const OOB* oob, uint8_t* buffer, size_t length, int port);

	std::string toString (const IPaddress& ipaddress) const;
public:
	Network ();
	virtual ~Network ();

	std::string getError (const std::string& prefix = "") const;

	void init () override;
	void shutdown () override;
	void update (uint32_t deltaTime) override;
	bool openServer (int port, IServerCallback* func) override;
	// the amount of clients that received the message
	int sendToClients (int clientMask, const IProtocolMessage& msg) override;
	void closeServer () override;
	void disconnectClientFromServer (ClientId clientId) override;
	bool isServer () const override;
	bool isClient () const override;
	bool openClient (const std::string& node, int port, IClientCallback* func) override;
	// return the amount of bytes transfered to the server
	int sendToServer (const IProtocolMessage& msg) override;
	void closeClient () override;
	bool isClientConnected () override;
	bool broadcast (IClientCallback* oobCallback, uint8_t* buffer, size_t length, int port) override;
};

inline bool Network::isClientConnected ()
{
	return _clientSocket != nullptr;
}

#endif
