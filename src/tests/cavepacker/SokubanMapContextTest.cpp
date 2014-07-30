#include "tests/cavepacker/SokubanMapContextTest.h"
#include "cavepacker/server/map/SokubanMapContext.h"

TEST(SokubanMapContextTest, testMapCreation)
{
	for (int i = 1; i <= 90; ++i) {
		const std::string name = String::format("%04i", i);
		SokubanMapContext ctx(name);
		ASSERT_TRUE(ctx.load(false));
	}
}
