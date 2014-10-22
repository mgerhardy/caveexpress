#pragma once
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "TestFrontend.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/campaign/ICampaignManager.h"
#include "engine/GameRegistry.h"
#include "engine/common/network/INetwork.h"

class TestCampaignMgr : public ICampaignManager {
public:
	CampaignPtr getActiveCampaign() const override {
		return CampaignPtr();
	}

	void reset() override {
	}

	bool updateMapValues(const std::string& mapname, uint32_t finishPoints,
			uint32_t time, uint8_t stars, bool lowerPointsAreBetter = false) override {
		return true;
	}
};

class MapSuite: public ::testing::Test {
protected:
	TestCampaignMgr _testCampaignMgr;
	TestFrontend _testFrontend;
	ServiceProvider _serviceProvider;
	EventHandler _eventHandler;

	virtual ~MapSuite() {}
	virtual void SetUp() override;
	virtual void TearDown() override;
};
