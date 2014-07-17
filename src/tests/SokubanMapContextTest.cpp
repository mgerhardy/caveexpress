#include "tests/RandomMapContextTest.h"
#include "cavepacker/server/map/SokubanMapContext.h"

TEST(SokubanMapContextTest, testMapCreation)
{
	SokubanMapContext ctx("map-1");
	ASSERT_TRUE(ctx.load(false));
}
