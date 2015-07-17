#include "TestShared.h"
#include "network/ProtocolHandlerRegistry.h"

void AbstractTest::SetUp() {
	const bool verbose = Config.getConfigVar("verbose", "false")->getBoolValue();
	if (!verbose)
		SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL);
	_serviceProvider.init(&_testFrontend);
	_serviceProvider.updateNetwork(false);
	Singleton<GameRegistry>::getInstance().getGame()->init(&_testFrontend, _serviceProvider);
}

void AbstractTest::TearDown() {
	ProtocolHandlerRegistry::get().shutdown();
}
