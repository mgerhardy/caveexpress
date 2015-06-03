#pragma once
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "TestFrontend.h"
#include "common/ServiceProvider.h"
#include "campaign/ICampaignManager.h"
#include "GameRegistry.h"
#include "network/INetwork.h"

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

class AbstractTest: public ::testing::Test {
protected:
	TestCampaignMgr _testCampaignMgr;
	TestFrontend _testFrontend;
	ServiceProvider _serviceProvider;
	EventHandler _eventHandler;

	virtual ~AbstractTest() {}
	virtual void SetUp() override;
	virtual void TearDown() override;
};
