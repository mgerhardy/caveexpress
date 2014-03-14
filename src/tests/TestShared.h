#pragma once
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "TestFrontend.h"
#include "engine/common/ServiceProvider.h"
#include "engine/GameRegistry.h"

class MapSuite: public ::testing::Test {
protected:
	TestFrontend _testFrontend;
	ServiceProvider _serviceProvider;
	EventHandler _eventHandler;

	virtual void SetUp() override;
	virtual void TearDown() override;
};

class TestGame : public IGame {
};

static GameRegisterStatic CAVEEXPRESS("test", GamePtr(new TestGame()));
