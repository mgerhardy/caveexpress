#include "tests/TestShared.h"
#include "campaign/Campaign.h"
#include "cavepacker/shared/CavePackerSQLitePersister.h"

TEST(CampaignTest, testNew) {
	CavePackerSQLitePersister *p = new CavePackerSQLitePersister("test/cavepacker.new.test");
	delete p;
}

TEST(CampaignTest, testLoad) {
	SCOPED_TRACE("test/cavepacker.gamestate.sqlite");
	SQLitePersister persister("test/cavepacker.gamestate.sqlite");
	Campaign campaign("tutorial", &persister);
	ASSERT_TRUE(campaign.loadProgress()) << "failed to load the campaign progress";
}
