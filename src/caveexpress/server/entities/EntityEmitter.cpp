#include "EntityEmitter.h"
#include "caveexpress/server/map/Map.h"
#include "engine/common/KeyValueParser.h"
#include "caveexpress/shared/constants/EmitterSettings.h"
#include "caveexpress/server/entities/npcs/NPCBlowing.h"
#include "caveexpress/server/entities/npcs/NPCAttacking.h"
#include "caveexpress/server/entities/Tree.h"
#include "caveexpress/server/entities/Fruit.h"
#include "caveexpress/server/entities/Egg.h"
#include "caveexpress/server/entities/Bomb.h"
#include "caveexpress/server/entities/Stone.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/PackageTarget.h"

EntityEmitter::EntityEmitter (Map& map, float gridX, float gridY, int amount, int delay, const EntityType& type, const std::string& settings) :
		IEntity(EntityTypes::EMITTER, map), _x(gridX), _y(gridY), _amount(amount), _delay(delay), _spawnTimer(delay), _type(
				type), _count(0), _settings(settings)
{
	update(0);
}

EntityEmitter::~EntityEmitter ()
{
}

bool EntityEmitter::shouldCollide (const IEntity* entity) const
{
	return false;
}

b2Vec2 EntityEmitter::getRealPos (const EntityType &entityType) const
{
	const b2Vec2 size(entityType.width, entityType.height);
	b2Vec2 pos;
	// TODO: _x and _y is the gridpos - not the center of the emitter body.
	pos.x = _x + size.x / 2.0f;
	pos.y = _y + ceilf(size.y - EPSILON) - size.y / 2.0f;
	return pos;
}

void EntityEmitter::update (uint32_t deltaTime)
{
	if (_amount > 0 && _count >= _amount)
		return;

	IEntity::update(deltaTime);

	_spawnTimer -= deltaTime;

	if (_spawnTimer > 0)
		return;

	_spawnTimer = _delay;

	const b2Vec2 realPos = getRealPos(_type);

	debug(LOG_SERVER, _type.name + " spawning");
	if (EntityTypes::isStone(_type)) {
		Stone* entity = new Stone(_map, realPos.x, realPos.y);
		entity->createBody();
	} else if (EntityTypes::isPackage(_type)) {
		Package* entity = new Package(_map, realPos.x, realPos.y);
		entity->createBody();
	} else if (EntityTypes::isNpcAttacking(_type)) {
		const KeyValueParser s(_settings);
		NPCAttacking* npc = _map.createAttackingNPC(realPos, _type, s.getBool(EMITTER_RIGHT, true));
		if (EntityTypes::isNpcMammut(_type)) {
			npc->setDazedTimeout(9000);
		}
	} else if (EntityTypes::isNpcBlowing(_type)) {
		const float strength = 10.0f;
		const float modificatorSize = 2.0f;
		const KeyValueParser s(_settings);
		_map.createBlowingNPC(realPos, s.getBool(EMITTER_RIGHT, true), s.getFloat(EMITTER_STRENGTH, strength), s.getFloat(EMITTER_WIND_MOD_SIZE, modificatorSize));
	} else if (EntityTypes::isBomb(_type)) {
		Bomb* entity = new Bomb(_map, realPos.x, realPos.y);
		entity->createBody();
	} else if (EntityTypes::isTree(_type)) {
		Tree* entity = new Tree(_map, realPos.x, realPos.y);
		entity->createBody();
	} else if (EntityTypes::isApple(_type)) {
		Fruit* entity = new Fruit(_map, EntityTypes::APPLE, realPos.x, realPos.y);
		entity->createBody();
	} else if (EntityTypes::isBanana(_type)) {
		Fruit* entity = new Fruit(_map, EntityTypes::BANANA, realPos.x, realPos.y);
		entity->createBody();
	} else if (EntityTypes::isEgg(_type)) {
		Egg* entity = new Egg(_map, realPos.x, realPos.y);
		entity->createBody();
	} else {
		error(LOG_SERVER, _type.name + " is an unknown type for the emitter");
		_amount = _count = 1;
		return;
	}

	_count++;
}

// we will remove the emitter if the max amount of entities were spawned
bool EntityEmitter::isRemove () const
{
	return _amount > 0 && _count >= _amount;
}

const EntityType** EntityEmitter::getSupportedEntityTypes()
{
	static const EntityType* types[] = { &EntityTypes::STONE, &EntityTypes::PACKAGE_ROCK, &EntityTypes::PACKAGE_ICE,
			&EntityTypes::NPC_WALKING, &EntityTypes::NPC_BLOWING, &EntityTypes::BANANA,
			&EntityTypes::APPLE, &EntityTypes::EGG, &EntityTypes::NPC_MAMMUT, &EntityTypes::TREE, &EntityTypes::BOMB, 0 };
	return types;
}
