#pragma once

#include "tests/TestShared.h"
#include "caveexpress/main/CaveExpress.h"
#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/server/entities/Bomb.h"
#include "caveexpress/server/entities/Egg.h"
#include "caveexpress/server/entities/Fruit.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/Stone.h"
#include "caveexpress/server/entities/Tree.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/npcs/NPCFish.h"
#include "caveexpress/server/entities/npcs/NPCFlying.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "caveexpress/server/map/Map.h"
#include "PhysicsSandboxMap.h"
#include "caveexpress/shared/CaveExpressConfig.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "caveexpress/shared/constants/NPCState.h"
#include "caveexpress/shared/constants/TreeState.h"
#include "common/ConfigManager.h"
#include "common/Shared.h"
#include "common/SpriteDefinition.h"
#include "common/TextureDefinition.h"
#include "network/INetwork.h"
#include <cmath>

namespace caveexpress {

class PhysicsTest: public AbstractTest {
protected:
	PhysicsSandboxMap _map;
	TextureDefinition* _textures = nullptr;

	void SetUp () override
	{
		AbstractTest::SetUp();
		Config.getConfigVar(GOD_MODE)->setValue("false");
		_serviceProvider.getNetwork().openServer(12345, nullptr);
		_textures = new TextureDefinition("small");
		SpriteDefinition::get().init(*_textures);
		registerCaveExpressConfigVars();
		_map.init(&_testFrontend, _serviceProvider);
		Config.getConfigVar(MAX_HITPOINTS)->setValue("100");
		Config.getConfigVar(DAMAGE_THRESHOLD)->setValue("0.3");
		ASSERT_TRUE(_map.initPhysicsSandbox(20, 16, 9.81f, 1.0f));
	}

	void TearDown () override
	{
		_map.shutdown();
		delete _textures;
		_textures = nullptr;
		AbstractTest::TearDown();
	}

	void tick (int steps = 1)
	{
		// Map::update only steps when elapsed time is >= DELTA_PHYSICS_MILLIS
		// (16.666...). Truncating that to 16 never enters the physics loop.
		const uint32_t dt = static_cast<uint32_t>(std::ceil(Constant::DELTA_PHYSICS_MILLIS));
		for (int i = 0; i < steps; ++i)
			_map.update(dt);
	}

	void freeze (IEntity* entity, const PhysicsVec2& pos)
	{
		entity->setGravityScale(0.0f);
		entity->setPos(pos);
		// Box2D 3 skips new contacts on sleeping bodies; keep a tiny motion.
		entity->setLinearVelocity(PhysicsVec2(0.05f, 0.05f));
		entity->applyLinearImpulse(PhysicsVec2(0.001f, 0.001f));
	}

	int countType (const EntityType& type)
	{
		int n = 0;
		struct Counter: IEntityVisitor {
			int& n;
			explicit Counter (int& c) :
					n(c)
			{
			}
			bool visitEntity (IEntity*) override
			{
				++n;
				return false;
			}
		} counter(n);
		_map.visitEntities(&counter, type);
		return n;
	}

	int countFruit ()
	{
		return countType(EntityTypes::APPLE) + countType(EntityTypes::BANANA);
	}

	template<class T>
	T* findType (const EntityType& type)
	{
		struct Finder: IEntityVisitor {
			T* found = nullptr;
			bool visitEntity (IEntity* entity) override
			{
				if (found == nullptr)
					found = static_cast<T*>(entity);
				return false;
			}
		} finder;
		_map.visitEntities(&finder, type);
		return finder.found;
	}

	MapTile* addGround (float x, float y)
	{
		return _map.addTileScripted("tile-ground-01", x, y);
	}

	void addGroundRow (float y, int x0, int x1)
	{
		for (int x = x0; x <= x1; ++x)
			addGround(static_cast<float>(x), y);
	}

	Tree* addTree (float x, float y)
	{
		Tree* tree = new Tree(_map, x, y);
		tree->createBody();
		return tree;
	}

	Stone* addStone (float x, float y, float vx, float vy)
	{
		Stone* stone = new Stone(_map, x, y, nullptr);
		stone->createBody();
		stone->setLinearVelocity(PhysicsVec2(vx, vy));
		return stone;
	}

	Package* addPackage (float x, float y)
	{
		return _map.spawnPackageScripted(x, y);
	}

	Fruit* addFruit (const EntityType& type, float x, float y)
	{
		Fruit* fruit = new Fruit(_map, type, x, y);
		fruit->createBody();
		return fruit;
	}

	Player* addPlayer (float x, float y)
	{
		return _map.spawnPlayerAt(x, y);
	}

	Egg* addEgg (float x, float y)
	{
		Egg* egg = new Egg(_map, x, y);
		egg->createBody();
		return egg;
	}

	Bomb* addBomb (float x, float y)
	{
		Bomb* bomb = new Bomb(_map, x, y);
		bomb->createBody();
		return bomb;
	}
};

}
