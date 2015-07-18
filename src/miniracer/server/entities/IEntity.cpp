#include "IEntity.h"
#include "miniracer/server/map/Map.h"
#include "common/SpriteDefinition.h"
#include "common/ConfigManager.h"
#include "miniracer/shared/EntityStates.h"

namespace miniracer {

uint32_t IEntity::GLOBAL_ENTITY_NUM = 0;

IEntity::IEntity (const EntityType &type, Map& map, float x, float y) :
		_id(GLOBAL_ENTITY_NUM++), _type(type), _time(0), _map(map), _state(MiniRacerEntityStates::NONE), _x(x), _y(y)
{
	setSpriteID("");
}

IEntity::~IEntity ()
{
}

SpriteDefPtr IEntity::getSpriteDef () const
{
	const std::string& spriteName = SpriteDefinition::get().getSpriteName(_type, Animation::NONE);
	const SpriteDefPtr& def = SpriteDefinition::get().getSpriteDefinition(spriteName);
	return def;
}

float IEntity::getAngle () const
{
	return 0.0f;
}

uint8_t IEntity::setState (uint8_t state)
{
	const uint8_t currentState = _state;
	_state = state;
	_map.updateEntity(0, *this);
	return currentState;
}

void IEntity::update (uint32_t deltaTime)
{
	_time += deltaTime;
	for (EntityObservers::iterator i = _observers.begin(); i != _observers.end(); ++i) {
		(*i)->onUpdate(this);
	}
}

void IEntity::remove ()
{
}

void IEntity::onSpawn ()
{
}

}
