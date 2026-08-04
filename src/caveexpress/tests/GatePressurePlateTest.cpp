#include "tests/TestShared.h"
#include "tests/AbstractProtocolTest.h"
#include "caveexpress/shared/network/messages/GateStateMessage.h"
#include "caveexpress/shared/CaveExpressMapContext.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/server/entities/Gate.h"
#include "caveexpress/server/entities/PressurePlate.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/main/CaveExpress.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"

namespace caveexpress {

TEST(GatePressurePlateTypesTest, testTypeHelpers)
{
	ASSERT_TRUE(SpriteTypes::isGate(SpriteTypes::GATE));
	ASSERT_TRUE(SpriteTypes::isPressurePlate(SpriteTypes::PRESSUREPLATE));
	ASSERT_TRUE(EntityTypes::isGate(EntityTypes::GATE));
	ASSERT_TRUE(EntityTypes::isPressurePlate(EntityTypes::PRESSUREPLATE));
	ASSERT_TRUE(EntityTypes::isSolid(EntityTypes::GATE));
	ASSERT_TRUE(EntityTypes::isSolid(EntityTypes::PRESSUREPLATE));
	ASSERT_TRUE(EntityTypes::isMapTile(EntityTypes::PRESSUREPLATE));
}

class GateStateProtocolTest: public AbstractProtocolTest {
};

TEST_F(GateStateProtocolTest, testGateStateMessage)
{
	testMessage("GateStateMessage", GateStateMessage(42, 200));
	testMessage("GateStateMessageZero", GateStateMessage(1, 0));
	testMessage("GateStateMessageFull", GateStateMessage(7, 255));
}

class FindGatePlateVisitor: public IEntityVisitor {
public:
	Gate* gate = nullptr;
	PressurePlate* plate = nullptr;
	bool visitEntity (IEntity *entity) override
	{
		if (entity->isGate())
			gate = static_cast<Gate*>(entity);
		else if (entity->isPressurePlate())
			plate = static_cast<PressurePlate*>(entity);
		return false;
	}
};

class GatePressurePlateMapTest: public AbstractTest {
protected:
	CaveExpress _game;
	TextureDefinition* _textures = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		_serviceProvider.getNetwork().openServer(12345, nullptr);
		_game.init(&_testFrontend, _serviceProvider);
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
	}

	void TearDown () override
	{
		_game.shutdown();
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}
};

TEST_F(GatePressurePlateMapTest, testLoadLinksAndOpens)
{
	ASSERT_TRUE(_game.mapLoad("tutorial-gate-01")) << "could not load tutorial-gate-01";
	Map& map = _game.getMap();

	FindGatePlateVisitor visitor;
	map.visitEntities(&visitor);
	ASSERT_NE(nullptr, visitor.gate);
	ASSERT_NE(nullptr, visitor.plate);
	EXPECT_EQ("door1", visitor.gate->getLinkId());
	EXPECT_EQ("door1", visitor.plate->getLinkId());
	ASSERT_NE(nullptr, visitor.plate->getLinkedGate());
	EXPECT_EQ(visitor.gate, visitor.plate->getLinkedGate());
	EXPECT_FALSE(visitor.gate->isOpenRequested());
	EXPECT_NEAR(1.0f, visitor.gate->getProtrusion(), 0.001f);

	visitor.gate->setOpen(true);
	EXPECT_TRUE(visitor.gate->isOpenRequested());
	for (int i = 0; i < 120; ++i)
		visitor.gate->update(16);
	EXPECT_LT(visitor.gate->getProtrusion(), 0.05f);
	EXPECT_EQ(0, visitor.gate->getProtrusionByte());

	visitor.gate->setOpen(false);
	for (int i = 0; i < 120; ++i)
		visitor.gate->update(16);
	EXPECT_NEAR(visitor.gate->getMaxOpenAmount(), visitor.gate->getProtrusion(), 0.05f);
}

class GatePressurePlateMapContextTest: public AbstractTest {
protected:
	TextureDefinition* _textures = nullptr;
	void SetUp () override
	{
		AbstractTest::SetUp();
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
	}
	void TearDown () override
	{
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}
};

TEST_F(GatePressurePlateMapContextTest, testLuaRoundTrip)
{
	CaveExpressMapContext ctx("tutorial-gate-01");
	ASSERT_TRUE(ctx.load(true));
	ASSERT_FALSE(ctx.getGateDefinitions().empty());
	ASSERT_FALSE(ctx.getPressurePlateDefinitions().empty());
	EXPECT_EQ("door1", ctx.getGateDefinitions().front().linkId);
	EXPECT_EQ("door1", ctx.getPressurePlateDefinitions().front().linkId);
	EXPECT_NEAR(700.0f, ctx.getPressurePlateDefinitions().front().requiredWeight, 0.01f);
	EXPECT_NEAR(1.0f, ctx.getGateDefinitions().front().openAmount, 0.01f);
	EXPECT_TRUE(SpriteDefinition::get().getSpriteDefinition("tile-gate-rock-01").get());
	EXPECT_TRUE(SpriteDefinition::get().getSpriteDefinition("tile-plate-01-idle").get());
	EXPECT_TRUE(SpriteDefinition::get().getSpriteDefinition("tile-plate-01-active").get());
}

}
