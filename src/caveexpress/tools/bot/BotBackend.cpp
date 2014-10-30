#include "BotBackend.h"
#include "engine/common/Logger.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/network/messages/PingMessage.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/FileSystem.h"
#include "engine/server/commands/CmdQuit.h"
#include "network/LoadMapHandler.h"
#include "network/MapRestartHandler.h"
#include "network/UpdateEntityHandler.h"
#include "network/AddEntityHandler.h"
#include "network/MapSettingsHandler.h"
#include "network/WaterHeightHandler.h"
#include "network/CloseMapHandler.h"
#include "network/UpdateHitpointsHandler.h"
#include "network/UpdateLivesHandler.h"
#include "network/UpdateHitpointsHandler.h"
#include "network/UpdateCollectedTypeHandler.h"
#include "network/RemoveEntityHandler.h"
#include "network/UpdatePackageCountHandler.h"
#include "network/AddCaveHandler.h"
#include "network/InitDoneHandler.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/common/Commands.h"
#include <SDL.h>

BotBackend::BotBackend () :
		_running(true), _frontend(_console)
{
	_eventHandler.registerObserver(this);
	Commands.registerCommand(CMD_QUIT, new CmdQuit());
	Commands.registerCommand("connect", bindFunction(BotBackend, connect));
	Commands.registerCommand("ping", bindFunction(BotBackend, ping));
}

BotBackend::~BotBackend ()
{
	_eventHandler.removeObserver(this);
	Commands.removeCommand(CMD_QUIT);
}

void BotBackend::ping ()
{
	if (!_serviceProvider.getNetwork().discover(this, Config.getPort()))  {
		error(LOG_BACKEND, "could not send the ping broadcast");
	} else {
		info(LOG_BACKEND, "sent ping broadcast");
	}
}

void BotBackend::connect (const ICommand::Args& args)
{
	if (args.size() < 1) {
		error(LOG_BACKEND, "usage: host <port>");
		return;
	}
	const String& host = args[0];
	const int port = args.size() == 2 ? args[1].toInt() : Config.getConfigVar("port")->getIntValue();
	_serviceProvider.getNetwork().openClient(host.c_str(), port, this);
}

void BotBackend::onOOBData (const std::string& host, const IProtocolMessage* message)
{
	if (message->getId() != protocol::PROTO_PING) {
		error(LOG_BACKEND, "got invalid discover broadcast reply");
		return;
	}
	const PingMessage* p = static_cast<const PingMessage*>(message);
	std::stringstream ss;
	ss << p->getName() << ": " << p->getMapName() << p->getPlayerCount() << "/" << p->getMaxPlayerCount();
	info(LOG_BACKEND, ss.str());
}

void BotBackend::onData (ByteStream &data)
{
	ProtocolMessageFactory& factory = ProtocolMessageFactory::get();
	while (factory.isNewMessageAvailable(data)) {
		// remove the size from the stream
		data.readShort();
		const ScopedPtr<IProtocolMessage> msg(factory.create(data));
		if (!msg) {
			error(LOG_BACKEND, "no message for type " + string::toString(static_cast<int>(data.readByte())));
			continue;
		}

		IClientProtocolHandler* handler = ProtocolHandlerRegistry::get().getClientHandler(*msg);
		if (handler != nullptr)
			handler->execute(*msg);
#if 0
		else
			debug(LOG_BACKEND, String::format("no client handler for message type %i", msg->getId()));
#endif
	}
}

int BotBackend::init (int argc, char **argv)
{
	srand(SDL_GetTicks());

	const int frontendInit = _frontend.init(0, 0, true, _eventHandler);
	Config.get().init(nullptr, argc, argv);

	_serviceProvider.init(&_frontend);
	_frontend.initUI(_serviceProvider);
	_frontend.onStart();

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(protocol::PROTO_LOADMAP, new LoadMapHandler(_serviceProvider));
	r.registerClientHandler(protocol::PROTO_MAPRESTART, new MapRestartHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEENTITY, new UpdateEntityHandler());
	r.registerClientHandler(protocol::PROTO_ADDENTITY, new AddEntityHandler());
	r.registerClientHandler(protocol::PROTO_MAPSETTINGS, new MapSettingsHandler());
	r.registerClientHandler(protocol::PROTO_WATERHEIGHT, new WaterHeightHandler());
	r.registerClientHandler(protocol::PROTO_CLOSEMAP, new CloseMapHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEHITPOINTS, new UpdateHitpointsHandler());
	r.registerClientHandler(protocol::PROTO_UPDATELIVES, new UpdateLivesHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEPACKAGECOUNT, new UpdatePackageCountHandler());
	r.registerClientHandler(protocol::PROTO_INITDONE, new InitDoneHandler());
	r.registerClientHandler(protocol::PROTO_UPDATECOLLECTEDTYPE, new UpdateCollectedTypeHandler());
	r.registerClientHandler(protocol::PROTO_REMOVEENTITY, new RemoveEntityHandler());
	r.registerClientHandler(protocol::PROTO_ADDCAVE, new AddCaveHandler());

	ping();

	return frontendInit;
}

void BotBackend::mainLoop (int argc, char **argv)
{
	if (init(argc, argv) == -1) {
		System.exit("Initialization error", EXIT_FAILURE);
	}

	info(LOG_BACKEND, "initialization done");

	while (_running) {
		runFrame();

		SDL_Delay(1);
	}

	Config.shutdown();
	_serviceProvider.getNetwork().shutdown();
}

void BotBackend::runFrame ()
{
	static uint32_t lastFrame = SDL_GetTicks() - 2;

	const uint32_t now = SDL_GetTicks();
	const uint32_t deltaTime = now - lastFrame;
	lastFrame = now;

	_serviceProvider.getNetwork().update(deltaTime);

	_frontend.update(deltaTime);
	_frontend.render();
}
