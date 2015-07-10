#pragma once

#include "common/Log.h"
#include "common/Timer.h"
#include "common/IMap.h"
#include "miniracer/shared/MiniRacerEntityType.h"
#include "common/Direction.h"
#include "common/EntityAlignment.h"
#include <memory>
#include "common/Math.h"
#include <vector>
#include <list>
#include <string>
#include <stdint.h>

// forward decl
class SpriteDef;
typedef std::shared_ptr<SpriteDef> SpriteDefPtr;

namespace miniracer {

class Map;
class IEntity;

class IEntityObserver {
public:
	virtual ~IEntityObserver ()
	{
	}

	// callback that is called each time the update method of
	// a physics entity is called
	virtual void onUpdate (IEntity *entity)
	{
	}
};

class IEntity {
protected:
	std::vector<std::string> _spriteIDs;
	uint16_t _id;
	const EntityType &_type;
	uint32_t _time;
	static uint32_t GLOBAL_ENTITY_NUM;

	typedef std::vector<IEntityObserver*> EntityObservers;
	EntityObservers _observers;

	Map& _map;
	uint8_t _state;

public:
	IEntity (const EntityType &type, Map& map);

	virtual ~IEntity ();

	// called when the entity is added to the world
	virtual void onSpawn ();

	// returns the angle of the entity in radians
	virtual float getAngle () const;

	inline uint8_t getState () const { return _state; }
	uint8_t setState (uint8_t state);

	virtual void update (uint32_t deltaTime);

	virtual void remove ();

	virtual SpriteDefPtr getSpriteDef () const;

	inline uint16_t getID () const
	{
		return _id;
	}

	inline const std::string& getSpriteID () const
	{
		return _spriteIDs[0];
	}

	inline void setSpriteID (const std::string& spriteID)
	{
		_spriteIDs.clear();
		addSpriteID(spriteID);
	}

	inline void addSpriteID (const std::string& spriteID)
	{
		_spriteIDs.push_back(spriteID);
	}

	inline Map& getMap()
	{
		return _map;
	}

	// register an observer for this entity
	void registerObserver (IEntityObserver *observer)
	{
		_observers.push_back(observer);
	}

	// remove a previously registered observer from this entity
	void removeObserver (IEntityObserver *observer)
	{
		// Traverse through the list and try to find the specified observer
		for (EntityObservers::iterator i = _observers.begin(); i != _observers.end();
		/* in-loop increment */) {
			if (*i == observer) {
				_observers.erase(i++);
				break;
			} else {
				++i;
			}
		}
	}

	// returns the type of the entity
	inline const EntityType& getType () const
	{
		return _type;
	}

	inline bool isPlayer () const
	{
		return EntityTypes::isPlayer(_type);
	}

	// true for entities that must be updated in the client (e.g. no fixed position)
	inline bool isDynamic () const
	{
		return EntityTypes::isDynamic(_type);
	}

	// checks whether the entity is solid. a solid entity will collide with other
	// entities by default
	inline bool isSolid () const
	{
		return EntityTypes::isSolid(_type);
	}
};

}
