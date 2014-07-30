#include "TestShared.h"
#include "engine/common/SpriteDefinition.h"

void MapSuite::SetUp() {
	_serviceProvider.init(&_testFrontend);
	SpriteDefinition::get().init(_serviceProvider.getTextureDefinition());
	UI::get().init(_serviceProvider, _eventHandler, _testFrontend);
}

void MapSuite::TearDown() {
	UI::get().shutdown();
}
