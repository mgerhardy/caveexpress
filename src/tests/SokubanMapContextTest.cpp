#include "tests/RandomMapContextTest.h"
#include "cavepacker/server/map/SokubanMapContext.h"

TEST(SokubanMapContextTest, testMapCreation)
{
	SokubanMapContext ctx("ice-random-1");
	ASSERT_TRUE(ctx.load(false));
}
