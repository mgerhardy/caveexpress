#include "tests/TestShared.h"
#include "miniracer/server/map/MiniRacerMapContext.h"
#include "common/ThemeType.h"

namespace miniracer {

TEST(MiniRacerMapContextTest, testLoad)
{
	MiniRacerMapContext ctxIceRandom1("map-0001");
	ASSERT_TRUE(ctxIceRandom1.load(false));
}

}
