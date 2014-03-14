#pragma once

#include "caveexpress/server/entities/IEntity.h"

class CollectableEntity: public IEntity {
protected:
	bool _collected;
	mutable const IEntity *_creator;
	uint32_t _collectedTime;
	IEntity *_collector;
	IEntity *_lastDropper;
	uint16_t _collectCount;

public:
	CollectableEntity (const EntityType& type, Map& map, const IEntity *creator = nullptr);

	virtual ~CollectableEntity ()
	{
	}
	void setCollected (bool collected, IEntity* collector);
	bool isCollected () const;
	IEntity* getCollector () const;
	IEntity* getLastDropper () const;
	uint32_t getCollectedTime () const;

	// IEntity
	virtual bool isRemove () const override;

	// IEntity
	virtual bool shouldCollide (const IEntity *entity) const override;
	virtual void onContact (b2Contact* contact, IEntity* entity) override;
};

inline void CollectableEntity::setCollected (bool collected, IEntity* collector)
{
	if (_collected == collected)
		return;

	_collected = collected;

	if (_collected) {
		_collectedTime = _time;
		_collector = collector;
		_lastDropper = nullptr;
		++_collectCount;
	} else {
		_collector = nullptr;
		_lastDropper = collector;
	}
}

inline bool CollectableEntity::isCollected () const
{
	return _collected;
}

inline IEntity* CollectableEntity::getCollector () const
{
	return _collector;
}

inline IEntity* CollectableEntity::getLastDropper () const
{
	return _lastDropper;
}

inline uint32_t CollectableEntity::getCollectedTime () const
{
	return _collectedTime;
}
