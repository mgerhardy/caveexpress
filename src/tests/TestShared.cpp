#include "TestShared.h"
#include "network/ProtocolHandlerRegistry.h"

void AbstractTest::SetUp() {
	const std::string& verbose = Config.getConfigVar("verbose", "")->getValue();
	if (!verbose.empty()) {
		ICommand::Args args;
		args.push_back(verbose);
		Config.setLogLevel(args);
	} else {
		SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL);
	}
	_serviceProvider.init(&_testFrontend);
	_serviceProvider.updateNetwork(false);
	Singleton<GameRegistry>::getInstance().getGame()->init(&_testFrontend, _serviceProvider);

	ProtocolHandlerRegistry& r = ProtocolHandlerRegistry::get();
	r.registerClientHandler(protocol::PROTO_CHANGEANIMATION, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_MAPRESTART, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_PAUSE, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEENTITY, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_REMOVEENTITY, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_SOUND, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_BACKTOMAIN, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_RUMBLE, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_PLAYERLIST, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_MESSAGE, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_CLOSEMAP, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_LOADMAP, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_MAPSETTINGS, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_INITWAITING, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_STARTMAP, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEHITPOINTS, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_UPDATELIVES, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_UPDATEPOINTS, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_TIMEREMAINING, new NopClientProtocolHandler());
	r.registerClientHandler(protocol::PROTO_FINISHEDMAP, new NopClientProtocolHandler());
}

void AbstractTest::TearDown() {
	ProtocolHandlerRegistry::get().shutdown();
}

const char* AbstractTest::va(const char* format, ...) {
	va_list argptr;
	static char string[4096];
	va_start(argptr, format);
	vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);
	return string;
}
