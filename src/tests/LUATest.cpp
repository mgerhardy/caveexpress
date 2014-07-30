#include "tests/LUATest.h"
#include "engine/common/Logger.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/TextureDefinition.h"
#include "engine/client/ui/FontDefinition.h"
#include "engine/common/EntityType.h"
#include "engine/common/Animation.h"

class LUATest: public MapSuite {
};

TEST_F(LUATest, testFontDefinition)
{
	FontDefinition d;
	ASSERT_FALSE(d.begin() == d.end()) << "no fonts found";
}

TEST_F(LUATest, testTextureDefinition)
{
	TextureDefinition small("small");
	ASSERT_FALSE(small.getMap().empty()) << "no texture definitions for small found";
	TextureDefinition big("big");
	ASSERT_FALSE(big.getMap().empty()) << "no texture definitions for big found";
}
