#include "tests/TestShared.h"
#include "campaign/Campaign.h"
#include "common/System.h"
#include "cavepacker/shared/CavePackerSQLitePersister.h"

namespace cavepacker {

TEST(CampaignTest, testNew) {
	CavePackerSQLitePersister *p = new CavePackerSQLitePersister(System.getDatabaseDirectory() + "cavepacker.new.test");
	ASSERT_TRUE(p->init()) << "Failed to initialize the persister";
	delete p;
}

TEST(CampaignTest, testLoad) {
	SCOPED_TRACE("cavepacker.gamestate.sqlite");
	SQLitePersister persister(System.getDatabaseDirectory() + "cavepacker.gamestate.sqlite");
	ASSERT_TRUE(persister.init()) << "Failed to initialize the persister";
	Campaign campaign("tutorial", &persister);
	ASSERT_TRUE(campaign.loadProgress()) << "failed to load the campaign progress";
}

}
