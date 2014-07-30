#include "tests/cavepacker/SokubanMapContextTest.h"
#include "cavepacker/server/map/SokubanMapContext.h"

TEST(SokubanMapContextTest, testMapCreation)
{
	SokubanMapContext ctx("0001");
	ASSERT_TRUE(ctx.load(false));
}
