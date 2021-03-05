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

Water::Water (Map& map, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay) :
		IEntity(EntityTypes::WATER, map), _waterChangeSpeed(waterChangeSpeed), _waterRisingDelay(waterRisingDelay), _waterFallingDelay(waterFallingDelay), _mapHeight(
				0.0f), _waterRisingState(WATER_UNINITIALIZED), _currentHeightLevel(0.0f), _waterRisingTime(waterRisingDelay), _waterFallingTime(waterFallingDelay)
{
	const b2Vec2 size(0.06f, 0.06f);
	if (Config.getConfigVar(WATER_PARTICLE)->getBoolValue())
		_waterParticle = new WorldParticle(map, WATER, 40, DENSITY_WATER / 1.05f, size, 1000);
	else
		_waterParticle = nullptr;
}

Water::~Water ()
{
}

SpriteDefPtr Water::getSpriteDef () const
{
	return SpriteDefPtr();
}

void Water::onContact (b2Contact* contact, IEntity* entity)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	IEntity* entityA = reinterpret_cast<IEntity*>(fixtureA->GetBody()->GetUserData().pointer);
	IEntity* entityB = reinterpret_cast<IEntity*>(fixtureB->GetBody()->GetUserData().pointer);
	const bool entityIsA = entityA == entity;
	const bool entityIsB = entityB == entity;

	if (entityIsB)
		_fixturePairs.insert(std::make_pair(fixtureA, fixtureB));
	else if (entityIsA)
		_fixturePairs.insert(std::make_pair(fixtureB, fixtureA));

	entity->setTouchingWater(true);
	_map.sendSound(entity->getVisMask(), SoundTypes::SOUND_WATER_IMPACT, entity->getPos());

	// TODO: this is not yet working - might have to do this in the pre solve step
	GameEvent.sendWaterImpact(entity->getPos().x, 1.0f);
	if (_waterParticle != nullptr)
		_waterParticle->addContact(entityIsA ? entityA : entityB);
}

void Water::endContact (b2Contact* contact, IEntity* entity)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	IEntity* entityA = reinterpret_cast<IEntity*>(fixtureA->GetBody()->GetUserData().pointer);
	IEntity* entityB = reinterpret_cast<IEntity*>(fixtureB->GetBody()->GetUserData().pointer);
	const bool entityIsA = entityA == entity;
	const bool entityIsB = entityB == entity;

	if (entityIsB)
		_fixturePairs.erase(std::make_pair(fixtureA, fixtureB));
	else if (entityIsA)
		_fixturePairs.erase(std::make_pair(fixtureB, fixtureA));

	_map.sendSound(entity->getVisMask(), SoundTypes::SOUND_WATER_LEAVE, entity->getPos());
	entity->setTouchingWater(false);
	if (_waterParticle != nullptr)
		_waterParticle->removeContact(entityIsA ? entityA : entityB);
}

void Water::updateFixtures ()
{
	for (FixturePairIter it = _fixturePairs.begin(); it != _fixturePairs.end(); ++it) {
		b2Fixture* waterFixture = it->first;
		SDL_assert(waterFixture->GetBody()->GetUserData().pointer == (uintptr_t)this);
		b2Fixture* entityFixture = it->second;
		const float density = waterFixture->GetDensity();
		const b2Body *waterBody = waterFixture->GetBody();
		b2Body *entityBody = entityFixture->GetBody();

		std::vector<b2Vec2>& points = getMap().getWaterIntersectionPoints();
		points.clear();
		if (!Buoyancy::findIntersectionOfFixtures(waterFixture, entityFixture, points)) {
			continue;
		}
		float area = 0;
		const b2Vec2 centroid = Buoyancy::computeCentroid(points, area);

		// apply buoyancy force
		const float displacedMass = waterFixture->GetDensity() * area;
		const b2Vec2 buoyancyForce = displacedMass * -getGravity();
		entityBody->ApplyForce(buoyancyForce, centroid, true);

		const int pointsSize = points.size();
		// apply drag separately for each polygon edge
		for (int i = 0; i < pointsSize; ++i) {
			// the end points and mid-point of this edge
			const b2Vec2& v0 = points[i];
			const b2Vec2& v1 = points[(i + 1) % pointsSize];
			const b2Vec2 midPoint = 0.5f * (v0 + v1);

			// find relative velocity between object and fluid at edge midpoint
			b2Vec2 velDir = entityBody->GetLinearVelocityFromWorldPoint(midPoint)
					- waterBody->GetLinearVelocityFromWorldPoint(midPoint);
			const float vel = velDir.Normalize();

			b2Vec2 edge = v1 - v0;
			const float edgeLength = edge.Normalize();
			const b2Vec2 normal = b2Cross(-1, edge); // gets perpendicular vector

			const float dragDot = b2Dot(normal, velDir);
			if (dragDot < 0)
				continue; // normal points backwards - this is not a leading edge

			const float dragMag = dragDot * edgeLength * density * vel * vel;
			const b2Vec2 dragForce = dragMag * -velDir;
			entityBody->ApplyForce(dragForce, midPoint, true);

			// apply lift
			const float liftDot = b2Dot(edge, velDir);
			const float liftMag = (dragDot * liftDot) * edgeLength * density * vel * vel;
			const b2Vec2 liftDir = b2Cross(1, velDir); // gets perpendicular vector
			const b2Vec2 liftForce = liftMag * liftDir;
			entityBody->ApplyForce(liftForce, midPoint, true);
		}
	}
}

void Water::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);

	updateFixtures();

	if (isWaterRisingDue()) {
		_waterRisingTime += 2 * _waterFallingDelay + 2 * _waterRisingDelay;
		_waterRisingState = WATER_RISING;
		setLinearVelocity(b2Vec2(0.0f, -_waterChangeSpeed));
	} else if (isWaterFallingDue()) {
		_waterFallingTime += 2 * _waterFallingDelay + 2 * _waterRisingDelay;
		_waterRisingState = WATER_FALLING;
		setLinearVelocity(b2Vec2(0.0f, _waterChangeSpeed));
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
	const float mapHeight = _map.getMapHeight();
	const float mapWidth = _map.getMapWidth();

	b2PolygonShape shape;
	shape.SetAsBox(mapWidth / 2.0f, mapHeight / 2.0f);

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.friction = 0.1f;
	fixture.restitution = 0.0f;
	fixture.density = DENSITY_WATER;
	fixture.isSensor = true;

	const float y = mapHeight - waterHeight;

	b2BodyDef bd;
	bd.position.Set(mapWidth / 2.0f, y + mapHeight / 2.0f);
	//bd.angle = DegreesToRadians(180.0f);
	bd.type = b2_kinematicBody;
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
