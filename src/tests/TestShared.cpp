#include "TestShared.h"
#include "network/ProtocolHandlerRegistry.h"

void AbstractTest::SetUp() {
	_serviceProvider.init(&_testFrontend);
	_serviceProvider.updateNetwork(false);
	Singleton<GameRegistry>::getInstance().getGame()->init(&_testFrontend, _serviceProvider);
}

void AbstractTest::TearDown() {
	ProtocolHandlerRegistry::get().shutdown();
}
