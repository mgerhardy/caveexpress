#include "Water.h"
#include "Buoyancy.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/Log.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "common/ConfigManager.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include <SDL_assert.h>

namespace caveexpress {

const float min_lastSoundDT = 1.0f;  // minimum time in seconds, between sounds

Water::Water (Map& map, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay) :
		IEntity(EntityTypes::WATER, map), _waterChangeSpeed(waterChangeSpeed), _waterRisingDelay(waterRisingDelay),
		_waterFallingDelay(waterFallingDelay), _mapHeight(0.0f), _waterRisingState(WATER_UNINITIALIZED),
		_currentHeightLevel(0.0f), _waterRisingTime(waterRisingDelay), _waterFallingTime(waterFallingDelay),
		_lastSoundDT(min_lastSoundDT)
{
	if (Config.getConfigVar(WORLD_PARTICLE)->getBoolValue()) {
		const PhysicsVec2 size(0.05f, 0.05f);
		_waterParticle = new WorldParticle(map, WATER, 200, DENSITY_WATER / 1.05f, size, 1000);
	}
}

Water::~Water ()
{
}

SpriteDefPtr Water::getSpriteDef () const
{
	return SpriteDefPtr();
}

void Water::onContact (PhysicsContact contact, IEntity* entity)
{
	PhysicsFixture fixtureA = contact.getFixtureA();
	PhysicsFixture fixtureB = contact.getFixtureB();
	IEntity* entityA = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
	IEntity* entityB = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());
	const bool entityIsA = entityA == entity;
	const bool entityIsB = entityB == entity;

	if (entityIsB)
		_fixturePairs.insert(std::make_pair(fixtureA, fixtureB));
	else if (entityIsA)
		_fixturePairs.insert(std::make_pair(fixtureB, fixtureA));

	entity->setTouchingWater(true);
	if (_lastSoundDT > min_lastSoundDT) {
		_lastSoundDT = 0.f;
		_map.sendSound(entity->getVisMask(), SoundTypes::SOUND_WATER_IMPACT, entity->getPos());
	}

	// TODO: this is not yet working - might have to do this in the pre solve step
	GameEvent.sendWaterImpact(entity->getPos().x, 1.0f);
	if (_waterParticle != nullptr)
		_waterParticle->addContact(entityIsA ? entityA : entityB);
}

void Water::endContact (PhysicsContact contact, IEntity* entity)
{
	PhysicsFixture fixtureA = contact.getFixtureA();
	PhysicsFixture fixtureB = contact.getFixtureB();
	IEntity* entityA = reinterpret_cast<IEntity*>(fixtureA.getBody().getUserData());
	IEntity* entityB = reinterpret_cast<IEntity*>(fixtureB.getBody().getUserData());
	const bool entityIsA = entityA == entity;
	const bool entityIsB = entityB == entity;

	if (entityIsB)
		_fixturePairs.erase(std::make_pair(fixtureA, fixtureB));
	else if (entityIsA)
		_fixturePairs.erase(std::make_pair(fixtureB, fixtureA));

	if (_lastSoundDT > min_lastSoundDT) {
		_lastSoundDT = 0.f;
		_map.sendSound(entity->getVisMask(), SoundTypes::SOUND_WATER_LEAVE, entity->getPos());
	}
	entity->setTouchingWater(false);
	if (_waterParticle != nullptr)
		_waterParticle->removeContact(entityIsA ? entityA : entityB);
}

void Water::updateFixtures ()
{
	for (FixturePairIter it = _fixturePairs.begin(); it != _fixturePairs.end(); ++it) {
		PhysicsFixture waterFixture = it->first;
		SDL_assert(waterFixture.getBody().getUserData() == (uintptr_t)this);
		PhysicsFixture entityFixture = it->second;
		const float density = waterFixture.getDensity();
		PhysicsBody waterBody = waterFixture.getBody();
		PhysicsBody entityBody = entityFixture.getBody();

		std::vector<PhysicsVec2>& points = getMap().getWaterIntersectionPoints();
		points.clear();
		if (!Buoyancy::findIntersectionOfFixtures(waterFixture, entityFixture, points)) {
			continue;
		}
		float area = 0;
		const PhysicsVec2 centroid = Buoyancy::computeCentroid(points, area);

		// apply buoyancy force
		const float displacedMass = waterFixture.getDensity() * area;
		const PhysicsVec2 buoyancyForce = displacedMass * -getGravity();
		entityBody.applyForce(buoyancyForce, centroid, true);

		const int pointsSize = points.size();
		// apply drag separately for each polygon edge
		for (int i = 0; i < pointsSize; ++i) {
			// the end points and mid-point of this edge
			const PhysicsVec2& v0 = points[i];
			const PhysicsVec2& v1 = points[(i + 1) % pointsSize];
			const PhysicsVec2 midPoint = 0.5f * (v0 + v1);

			// find relative velocity between object and fluid at edge midpoint
			PhysicsVec2 velDir = entityBody.getLinearVelocityFromWorldPoint(midPoint)
					- waterBody.getLinearVelocityFromWorldPoint(midPoint);
			const float maxVel = 4.0f;
			const float vel = std::min(maxVel, velDir.normalize());

			PhysicsVec2 edge = v1 - v0;
			const float edgeLength = edge.normalize();
			const PhysicsVec2 normal = physCross(-1, edge); // gets perpendicular vector

			const float dragDot = physDot(normal, velDir);
			if (dragDot < 0)
				continue; // normal points backwards - this is not a leading edge

			const float dragMag = dragDot * edgeLength * density * vel * vel;
			const PhysicsVec2 dragForce = dragMag * -velDir;
			entityBody.applyForce(dragForce, midPoint, true);

			// apply lift
			const float liftDot = physDot(edge, velDir);
			const float liftMag = (dragDot * liftDot) * edgeLength * density * vel * vel;
			const PhysicsVec2 liftDir = physCross(1, velDir); // gets perpendicular vector
			const PhysicsVec2 liftForce = liftMag * liftDir;
			entityBody.applyForce(liftForce, midPoint, true);
		}
	}
}

void Water::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);
	_lastSoundDT += (float)deltaTime / 1000.f;

	updateFixtures();

	if (isWaterRisingDue()) {
		_waterRisingTime += 2 * _waterFallingDelay + 2 * _waterRisingDelay;
		_waterRisingState = WATER_RISING;
		setLinearVelocity(PhysicsVec2(0.0f, -_waterChangeSpeed));
	} else if (isWaterFallingDue()) {
		_waterFallingTime += 2 * _waterFallingDelay + 2 * _waterRisingDelay;
		_waterRisingState = WATER_FALLING;
		setLinearVelocity(PhysicsVec2(0.0f, _waterChangeSpeed));
	}

	// not yet due
	if (_waterRisingState == WATER_UNINITIALIZED)
		return;

	// not enough movement to send an update again
	if (fequals(_currentHeightLevel, getPos().y, 0.01f))
		return;

	GameEvent.sendWaterUpdate(0, *this);
	_currentHeightLevel = getPos().y;
}

void Water::createBody (float waterHeight)
{
	const float mapHeight = (float)_map.getMapHeight();
	const float mapWidth = (float)_map.getMapWidth();

	PhysicsFixtureDef fixture;
	fixture.shapeType = PhysicsShapeType::Polygon;
	fixture.useBox = true;
	fixture.boxHalfWidth = mapWidth / 2.0f;
	fixture.boxHalfHeight = mapHeight / 2.0f;
	fixture.friction = 0.1f;
	fixture.restitution = 0.0f;
	fixture.density = DENSITY_WATER;
	fixture.isSensor = true;

	const float y = mapHeight - waterHeight;

	PhysicsBodyDef bd;
	bd.position.set(mapWidth / 2.0f, y + mapHeight / 2.0f);
	//bd.angle = DegreesToRadians(180.0f);
	bd.type = PhysicsBodyType::Kinematic;
	bd.fixedRotation = true;

	_map.addToWorld(fixture, bd, this);
	_map.addEntity(this);

	if (_waterParticle != nullptr)
		_map.addEntity(_waterParticle);

	_mapHeight = mapHeight / 2.0f;
}

bool Water::shouldCollide (const IEntity* entity) const
{
	return false;
}

}
