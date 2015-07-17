#include "tests/TestShared.h"
#include "miniracer/server/map/MiniRacerMapContext.h"
#include "common/ThemeType.h"

namespace miniracer {

TEST(MiniRacerMapContextTest, testLoad)
{
	MiniRacerMapContext ctx("map-0001");
	ASSERT_TRUE(ctx.load(false));
}

}
