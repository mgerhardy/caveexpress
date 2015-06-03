#include "tests/TestShared.h"
#include "cavepacker/server/map/SokobanMapContext.h"

TEST(SokobanMapContextTest, testMapCreation)
{
	for (int i = 1; i <= 90; ++i) {
		const std::string name = String::format("xsokoban%04i", i);
		SokobanMapContext ctx(name);
		ASSERT_TRUE(ctx.load(false));
	}
}
