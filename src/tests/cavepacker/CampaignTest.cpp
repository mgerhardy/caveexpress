#include "CampaignTest.h"
#include "engine/common/campaign/CampaignManager.h"
#include "cavepacker/shared/CavePackerSQLitePersister.h"

TEST(CampaignTest, testNew) {
	CavePackerSQLitePersister *p = new CavePackerSQLitePersister("test/cavepacker.new.test");
	delete p;
}
