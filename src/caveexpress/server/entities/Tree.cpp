#include "Tree.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/server/entities/Fruit.h"
#include "common/TimeManager.h"
#include "caveexpress/shared/constants/TreeState.h"

Tree::Tree (Map& map, gridCoord x, gridCoord y) :
		IEntity(EntityTypes::TREE, map), _x(x), _y(y), _dropFruit(false), _droppedFruits(0), _dropFruitCausedBy(nullptr), _idleTimer(0)
{
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
	b2PolygonShape sd;
	sd.SetAsBox(_size.x / 2.0f, _size.y / 2.0f);

	b2FixtureDef fd;
	fd.shape = &sd;
	fd.density = DENSITY_TREE;
	fd.friction = 0.0f;
	fd.restitution = 0.0f;
	fd.isSensor = true;

	b2BodyDef bd;
	bd.position.Set(_x, _y);
	bd.type = b2_staticBody;
	bd.fixedRotation = true;

	_map.addToWorld(fd, bd, this);
	_map.addEntity(this);
}

bool Tree::shouldCollide (const IEntity *entity) const
{
	if (getState() == TreeState::TREE_DAZED)
		return false;

	if (_dropFruitCausedBy == entity)
		return false;

	return entity->isStone();
}

void Tree::onContact (b2Contact* contact, IEntity* entity)
{
	IEntity::onContact(contact, entity);
	if (!entity->isStone())
		return;

	if (entity->getLinearVelocity().y > 0.1)
		setDazed(entity);
}

void Tree::endContact (b2Contact* contact, IEntity* entity)
{
	if (_dropFruitCausedBy == entity)
		_dropFruitCausedBy = nullptr;
}
