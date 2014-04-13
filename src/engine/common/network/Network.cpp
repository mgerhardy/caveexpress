#include "Network.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/Logger.h"
#include "engine/common/System.h"
#include <signal.h>
#include <SDL_net.h>

Network::Network () :
		INetwork(), _serverSocket(nullptr), _clientSocket(nullptr), _socketSet(nullptr), _clientId(0), _bytesIn(0), _bytesOut(
				0), _time(0), _closing(false)
{
#ifdef NET_USE_UDP
	_serverDatagramSocket = nullptr;
	_serverDatagramPacket = nullptr;
#endif

	_oobSockets.clear();

	init();
}

Network::~Network ()
{
}

void Network::init ()
{
	if (SDLNet_Init() < 0)
		System.exit(getError(), 1);

#ifdef NET_USE_UDP
	_serverDatagramSocket = nullptr;
	_serverDatagramPacket = nullptr;
#endif

	_socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS);
	if (!_socketSet)
		System.exit(getError(), 1);
}

void Network::disconnectClientFromServer (ClientId clientId)
{
	for (ClientSockets::iterator i = _clients.begin(); i != _clients.end(); ++i) {
		if ((*i)->num == clientId) {
			(*i)->disconnect = true;
			break;
		}
	}
}

void Network::closeServer ()
{
	if (!_socketSet)
		return;

	if (_serverSocket)
		info(LOG_NET, "close server");

	for (ClientSockets::iterator i = _clients.begin(); i != _clients.end(); ++i) {
		if (SDLNet_TCP_DelSocket(_socketSet, (*i)->socket) == -1)
			error(LOG_NET, getError());
	}

	SDLNet_TCP_DelSocket(_socketSet, _serverSocket);
	SDLNet_TCP_Close(_serverSocket);

#ifdef NET_USE_UDP
	SDLNet_UDP_Close(_serverDatagramSocket);
	SDLNet_FreePacket(_serverDatagramPacket);
	_serverDatagramSocket = nullptr;
	_serverDatagramPacket = nullptr;
#endif
	_serverSocket = nullptr;
	_clients.clear();
	_clientId = 0;
}

void Network::closeClient ()
{
	if (_closing)
		return;

	if (!_socketSet)
		return;

	_closing = true;
	if (isClientConnected()) {
		info(LOG_NET, "close client");
		const DisconnectMessage msg;
		sendToServer(msg);
	}
	SDLNet_TCP_DelSocket(_socketSet, _clientSocket);
	SDLNet_TCP_Close(_clientSocket);
	_clientSocket = nullptr;
}

bool Network::openServer (int port, IServerCallback* func)
{
	closeServer();

	info(LOG_NET, String::format("try to bind port %i", port));

	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, nullptr, port) < 0) {
		error(LOG_NET, getError("resolve host"));
		return false;
	}

	// open the listen socket
	if (!(_serverSocket = SDLNet_TCP_Open(&ip))) {
		error(LOG_NET, getError("tcp open"));
		return false;
	}

#ifdef NET_USE_UDP
	if (!(_serverDatagramSocket = SDLNet_UDP_Open(port))) {
		error(LOG_NET, getError("udp open"));
		SDLNet_TCP_Close(_serverSocket);
		_serverSocket = nullptr;
		return false;
	}

	const IPaddress address = { INADDR_BROADCAST, SDL_SwapBE16(port) };
	if (SDLNet_UDP_Bind(_serverDatagramSocket, -1, &address) < 0) {
		error(LOG_NET, getError("bind"));
		SDLNet_TCP_Close(_serverSocket);
		SDLNet_UDP_Close(_serverDatagramSocket);
		_serverSocket = nullptr;
		_serverDatagramSocket = nullptr;
		return false;
	}

	_serverDatagramPacket = SDLNet_AllocPacket(512);
#endif

	if (SDLNet_TCP_AddSocket(_socketSet, _serverSocket) < 0) {
		error(LOG_NET, getError("add socket"));
	}

	_serverFunc = func;

	return true;
}

bool Network::isServer () const
{
	return _serverSocket != nullptr;
}

bool Network::isClient () const
{
	return _clientSocket != nullptr;
}

void Network::handleDisconnectedClients ()
{
	for (ClientSockets::iterator i = _clients.begin(); i != _clients.end();) {
		ClientPtr client = *i;
		if (client->disconnect) {
			i = _clients.erase(i);
			info(LOG_NET, String::format("disconnect %i", client->num));
			if (SDLNet_TCP_DelSocket(_socketSet, client->socket) == -1)
				error(LOG_NET, getError());
			_serverFunc->onDisconnect(client->num);
		} else {
			++i;
		}
	}
}

bool Network::openClient (const std::string& node, int port, IClientCallback* func)
{
	closeClient();
	_closing = false;

	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, node.c_str(), port) < 0) {
		error(LOG_NET, getError());
		func->onConnectionError();
		return false;
	}

	// open the listen socket
	if (!(_clientSocket = SDLNet_TCP_Open(&ip))) {
		error(LOG_NET, getError());
		func->onConnectionError();
		return false;
	}

	if (SDLNet_TCP_AddSocket(_socketSet, _clientSocket) < 0) {
		error(LOG_NET, getError());
		func->onConnectionError();
		return false;
	}

	_clientFunc = func;

	func->onConnectionSuccess();

	return true;
}

void Network::shutdown ()
{
	INetwork::shutdown();
	for (OOBSockets::iterator i = _oobSockets.begin(); i != _oobSockets.end(); ) {
		OOB* oob = *i;
		i = _oobSockets.erase(i);
		delete oob;
	}
	_oobSockets.clear();
	SDLNet_FreeSocketSet(_socketSet);
	SDLNet_Quit();

	_closing = false;
	_socketSet = nullptr;
	_bytesIn = 0;
	_bytesOut = 0;
	info(LOG_NET, String::format("bytes received: %i, bytes sent: %i", _bytesIn, _bytesOut));
}

void Network::update (uint32_t deltaTime)
{
	_time += deltaTime;

#ifdef NET_USE_UDP
	if (_serverDatagramSocket) {
		if (SDLNet_UDP_Recv(_serverDatagramSocket, _serverDatagramPacket)) {
			_bytesIn += _serverDatagramPacket->len;
			const unsigned char *data = reinterpret_cast<const unsigned char *>(_serverDatagramPacket->data);
			const ProtocolMessagePtr p = _serverFunc->onOOBData(data);
			if (!sendUDP(_serverDatagramSocket, _serverDatagramPacket->address, *p))
				error(LOG_NET, "error sending udp response");
		}
	}

	for (OOBSockets::iterator i = _oobSockets.begin(); i != _oobSockets.end();) {
		OOB* oob = *i;
		if (oob->recv() == 0) {
			++i;
			continue;
		}
		_bytesIn += oob->getPacketLength();
		ByteStream s;
		oob->append(s);
		oob->onOOBData(s);
		i = _oobSockets.erase(i);
		delete oob;
	}
#endif

	const int ready = SDLNet_CheckSockets(_socketSet, 0);
	if (ready <= 0)
		return;

	if (deltaTime > 0 && (_time % 5000) == 0) {
		info(LOG_NET, String::format("sent: %i - received: %i", _bytesOut, _bytesIn));
	}

	if (_serverSocket) {
		if (SDLNet_SocketReady(_serverSocket)) {
			TCPsocket csd = SDLNet_TCP_Accept(_serverSocket);
			if (csd) {
				IPaddress *remoteIP = SDLNet_TCP_GetPeerAddress(csd);
				if (remoteIP) {
					if (SDLNet_TCP_AddSocket(_socketSet, csd) < 0) {
						error(LOG_NET, getError());
						SDLNet_TCP_Close(csd);
					} else {
						info(LOG_NET, String::format("connect %i", _clientId));
						_clients.push_back(ClientPtr(new Client(csd, _clientId)));
						_serverFunc->onConnection(_clientId);
						_clientId++;
					}
				} else {
					SDLNet_TCP_Close(csd);
				}
			}
		}

		for (ClientSockets::iterator i = _clients.begin(); i != _clients.end(); ++i) {
			ClientPtr c = *i;
			if (!SDLNet_SocketReady(c->socket))
				continue;

			(*i)->buf.clear();
			const int read = recv(c->socket, c->buf);
			if (read > 0) {
				_serverFunc->onData((*i)->num, (*i)->buf);
			} else {
				(*i)->disconnect = true;
			}
		}

		if (_clients.empty()) {
			// closeServer();
		}
	}

	if (isClientConnected()) {
		ByteStream buffer;
		const int read = recv(_clientSocket, buffer);
		if (read > 0) {
			_clientFunc->onData(buffer);
		}
	}

	handleDisconnectedClients();
}

int Network::send (TCPsocket socket, const ByteStream &buffer)
{
	const int size = buffer.getSize();
	const int written = SDLNet_TCP_Send(socket, buffer.getBuffer(), size);
	if (written == -1 || written != size) {
		error(LOG_NET, getError());
	} else {
		_bytesOut += written;
	}
	return written;
}

int Network::recv (TCPsocket socket, ByteStream &buffer)
{
	if (!SDLNet_SocketReady(socket))
		return 0;
	const size_t bufSize = 512;
	uint8_t buf[bufSize];
	int totalRead = 0;
	for (;;) {
		int read = SDLNet_TCP_Recv(socket, (void*) buf, bufSize);
		if (read <= -1) {
			error(LOG_NET, getError());
			return read;
		}
		if (read == 0) {
			break;
		}
		buffer.append(buf, read);
		totalRead += read;
		if (read < bufSize)
			break;
	}
	_bytesIn += totalRead;
	return totalRead;
}

int Network::sendToClients (int clientMask, const ByteStream& buffer)
{
	const int size = buffer.getSize();
	if (size == 0)
		return 0;
	int sent = 0;
	for (ClientSockets::iterator i = _clients.begin(); i != _clients.end(); ++i) {
		if (clientMask == 0 || (clientMask & ClientIdToClientMask((*i)->num))) {
			const int n = send((*i)->socket, buffer);
			if (n == size) {
				++sent;
			} else {
				(*i)->disconnect = true;
			}
		}
	}

	return sent;
}

int Network::sendToClients (int clientMask, const IProtocolMessage& msg)
{
	ByteStream s;
	msg.serialize(s);
	count(msg);
	return sendToClients(clientMask, s);
}

bool Network::sendUDP (UDPsocket sock, const IPaddress &address, const IProtocolMessage& msg)
{
#ifdef NET_USE_UDP
	ByteStream buffer;
	msg.serialize(buffer);
	count(msg);
	const size_t length = buffer.getSize();
	ScopedPtr<UDPpacket> p(SDLNet_AllocPacket(length));
	if (!p) {
		error(LOG_NET, getError());
		return false;
	}

	p->address = address;
	p->data = const_cast<uint8_t *>(buffer.getBuffer());
	p->len = length;

	const int numsent = SDLNet_UDP_Send(sock, -1, p);
	if (numsent <= 0) {
		error(LOG_NET, getError());
		return false;
	}

	_bytesOut += p->len;
	return true;
#else
	return false;
#endif
}

Network::OOB* Network::openOOB (IClientCallback* oobCallback, int16_t port)
{
#ifdef NET_USE_UDP
	UDPsocket sd = SDLNet_UDP_Open(port);
	if (sd == nullptr) {
		error(LOG_NET, getError());
		return nullptr;
	}

	OOB *oob = new OOB(sd, oobCallback, this);

	_oobSockets.push_back(oob);

	return oob;
#else
	error(LOG_NET, "failed to open oob");
	return nullptr;
#endif
}

bool Network::broadcast (IClientCallback* oobCallback, uint8_t* buffer, size_t length, int port)
{
	const OOB* oob = openOOB(oobCallback);
	return broadcast(oob, buffer, length, port);
}

bool Network::broadcast (const OOB* oob, uint8_t* buffer, size_t length, int port)
{
	if (!oob)
		return false;

	if (length == 0)
		return false;

#ifdef NET_USE_UDP
	UDPpacket* p = SDLNet_AllocPacket(length);
	if (!p) {
		error(LOG_NET, getError());
		return false;
	}

	p->address.host = INADDR_BROADCAST;
	p->address.port = SDL_SwapBE16(port);
	memcpy(p->data, buffer, length);
	p->len = length;

	const int numsent = oob->send(p);
	if (numsent <= 0) {
		error(LOG_NET, getError());
		SDLNet_FreePacket(p);
		return false;
	}

	_bytesOut += p->len;

	SDLNet_FreePacket(p);
	return true;
#else
	return false;
#endif
}

int Network::sendToServer (const ByteStream& buffer)
{
	if (!isClientConnected())
		return -1;

	const int n = send(_clientSocket, buffer);
	if (n == -1 || n != buffer.getSize())
		closeClient();

	return n;
}

int Network::sendToServer (const IProtocolMessage& msg)
{
	ByteStream s;
	msg.serialize(s);
	count(msg);
	return sendToServer(s);
}

std::string Network::getError (const std::string& prefix) const
{
	const char *netError = SDLNet_GetError();
	if (netError[0] == '\0') {
		return "";
	}
	std::string error = prefix;
	if (error.length() > 0) {
		error += " ";
	}
	error += netError;
	return error;
}

std::string Network::toString (const IPaddress &ipAddress) const
{
	Uint32 hostMask = ipAddress.host;
	const Uint32 converted = SDLNet_Read32(&hostMask);
	const unsigned char* host = (const unsigned char*) &converted;
	return String::format("%i.%i.%i.%i", host[3], host[2], host[1], host[0]).str();
}
