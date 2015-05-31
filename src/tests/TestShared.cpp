#include "TestShared.h"
#include "common/SpriteDefinition.h"
#include "network/ProtocolHandlerRegistry.h"

void MapSuite::SetUp() {
	_serviceProvider.init(&_testFrontend);
	_serviceProvider.updateNetwork(false);
	Singleton<GameRegistry>::getInstance().getGame()->init(&_testFrontend, _serviceProvider);
	SpriteDefinition::get().init(_serviceProvider.getTextureDefinition());
	UI::get().init(_serviceProvider, _eventHandler, _testFrontend);
}

void MapSuite::TearDown() {
	UI::get().shutdown();
	ProtocolHandlerRegistry::get().shutdown();
}

ProtocolMessageFactory::ProtocolMessageFactory ()
{
}

IProtocolMessage *ProtocolMessageFactory::create (ByteStream& stream) const
{
	return nullptr;
}
