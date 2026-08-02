#include "Tree.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/server/entities/Fruit.h"
#include "common/ConfigManager.h"
#include "common/TimeManager.h"
#include "caveexpress/shared/constants/TreeState.h"

namespace caveexpress {

Tree::Tree (Map& map, gridCoord x, gridCoord y) :
		IEntity(EntityTypes::TREE, map), _x(x), _y(y), _dropFruit(false), _droppedFruits(0), _dropFruitCausedBy(nullptr), _idleTimer(0)
{
	if (Config.getConfigVar(WORLD_PARTICLE)->getBoolValue()) {
		const PhysicsVec2 size(0.05f, 0.05f);
		_leafParticle = new WorldParticle(map, LEAF, 10, DENSITY_LEAF, size, 10000);
	}
	setIdle();
}

Tree::~Tree ()
{
	_map.getTimeManager().clearTimeout(_idleTimer);
}

void Tree::setIdle ()
{
	setAnimationType(Animations::ANIMATION_IDLE);
	setState(TreeState::TREE_IDLE);
	_idleTimer = 0;
}

void Tree::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);
	if (_dropFruit) {
		++_droppedFruits;
		const int randomFruit = rand() % 2;
		const EntityType *type;
		if (randomFruit == 0)
			type = &EntityTypes::APPLE;
		else
			type = &EntityTypes::BANANA;
		Fruit* fruit = new Fruit(_map, *type, _x, _y);
		fruit->createBody();
		_idleTimer = _map.getTimeManager().setTimeout(5000, this, &Tree::setIdle);
		_dropFruit = false;
	}
}

void Tree::setDazed (IEntity* entity)
{
	if (_dropFruitCausedBy == entity)
		return;

	if (getState() == TreeState::TREE_DAZED)
		return;

	if (entity) {
		_dropFruit = _droppedFruits < 10;
		setAnimationType(Animations::ANIMATION_DAZED);
		setState(TreeState::TREE_DAZED);
	}
	_dropFruitCausedBy = entity;
}

void Tree::createBody ()
{
	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Polygon;
	fd.useBox = true;
	fd.boxHalfWidth = _size.x / 2.0f;
	fd.boxHalfHeight = _size.y / 2.0f;
	fd.density = DENSITY_TREE;
	fd.friction = 0.0f;
	fd.restitution = 0.0f;
	fd.isSensor = true;

	PhysicsBodyDef bd;
	bd.position.set(_x, _y);
	bd.type = PhysicsBodyType::Static;
	bd.fixedRotation = true;

	_map.addToWorld(fd, bd, this);
	_map.addEntity(this);

	if (_leafParticle != nullptr)
		_map.addEntity(_leafParticle);
}

bool Tree::shouldCollide (const IEntity *entity) const
{
	if (getState() == TreeState::TREE_DAZED)
		return false;

	if (_dropFruitCausedBy == entity)
		return false;

	return entity->isStone();
}

void Tree::onContact (PhysicsContact contact, IEntity* entity)
{
	IEntity::onContact(contact, entity);
	if (!entity->isStone())
		return;

	if (entity->getLinearVelocity().y > 0.1)
		setDazed(entity);

	if (_leafParticle != nullptr) {
		PhysicsFixture fixtureA = contact.getFixtureA();
		PhysicsFixture fixtureB = contact.getFixtureB();
		IEntity* entityA = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
		IEntity* entityB = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());
		const bool entityIsA = entityA == entity;
		_leafParticle->addContact(entityIsA ? entityA : entityB);
	}
}

void Tree::endContact (PhysicsContact contact, IEntity* entity)
{
	if (_dropFruitCausedBy == entity)
		_dropFruitCausedBy = nullptr;
	if (_leafParticle != nullptr) {
		PhysicsFixture fixtureA = contact.getFixtureA();
		PhysicsFixture fixtureB = contact.getFixtureB();
		IEntity* entityA = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
		IEntity* entityB = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());
		const bool entityIsA = entityA == entity;
		_leafParticle->removeContact(entityIsA ? entityA : entityB);
	}
}

}
