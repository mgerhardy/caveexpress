#pragma once

#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/server/entities/WorldParticle.h"
#include <set>
#include <vector>

// forward decl
class Map;

typedef enum {
	WATER_UNINITIALIZED,
	WATER_RISING,
	WATER_FALLING
} WaterState;

class Water: public IEntity {
private:
	const float _waterChangeSpeed;
	const uint32_t _waterRisingDelay;
	const uint32_t _waterFallingDelay;
	float _mapHeight;
	WaterState _waterRisingState;
	float _currentHeightLevel;
	uint32_t _waterRisingTime;
	uint32_t _waterFallingTime;

	typedef std::pair<b2Fixture*, b2Fixture*> FixturePair;
	typedef std::set<FixturePair>::iterator FixturePairIter;
	std::set<FixturePair> _fixturePairs;

	WorldParticle *_waterParticle;

	void updateFixtures ();
public:
	Water (Map& map, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay);
	virtual ~Water ();

	float getHeight () const;
	bool isWaterRisingDue () const;
	bool isWaterFallingDue () const;

	// will return true if this map will at some point rise the water
	bool isWaterRisingEnabled () const;
	bool isWaterChangeOver () const;

	void createBody (float waterHeight);

	// IEntity
	SpriteDefPtr getSpriteDef () const override;
	void update (uint32_t deltaTime) override;
	void onContact (b2Contact* contact, IEntity* entity) override;
	void endContact (b2Contact* contact, IEntity* entity) override;
	bool shouldCollide (const IEntity* entity) const override;
};

inline float Water::getHeight () const
{
	return getPos().y - _mapHeight;
}

inline bool Water::isWaterRisingDue () const
{
	if (_time < _waterRisingTime)
		return false;
	return _waterChangeSpeed > 0.00001f;
}

inline bool Water::isWaterRisingEnabled () const
{
	return _waterChangeSpeed > 0.00001f;
}

inline bool Water::isWaterChangeOver () const
{
	return _time > _waterRisingTime && _time > _waterFallingTime;
}

inline bool Water::isWaterFallingDue () const
{
	if (_time < _waterFallingTime)
		return false;
	return _waterChangeSpeed > 0.00001f;
}
