#include "INetwork.h"
#include "common/Log.h"

INetwork::CountMap INetwork::_count;

INetwork::INetwork () :
		_clientFunc(nullptr), _serverFunc(nullptr)
{
}

INetwork::~INetwork ()
{
}

void INetwork::count (const IProtocolMessage &msg)
{
	CountMap::iterator i = _count.find(msg.getId());
	int value = 1;
	if (i != _count.end()) {
		value += i->second;
	}
	_count[msg.getId()] = value;
}

void INetwork::shutdown ()
{
	closeClient();
	closeServer();

	for (CountMap::iterator i = _count.begin(); i != _count.end(); ++i) {
		Log::info(LOG_NETWORK, "used protocol id %i %i times", static_cast<int>(i->first), i->second);
	}
	_count.clear();
}

int INetwork::sendToAllClients (const IProtocolMessage& msg)
{
	return sendToClients(0, msg);
}
