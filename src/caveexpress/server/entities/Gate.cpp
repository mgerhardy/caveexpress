#include "Gate.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "common/Math.h"
#include "common/vec2.h"
#include <algorithm>
#include <cmath>

namespace caveexpress {

namespace {
const float GATE_OPEN_THRESHOLD = 0.05f;
const float GATE_PROTRUSION_SPEED = 2.0f; // units per second
}

Gate::Gate (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY, const std::string& linkId,
		float openAmount) :
		MapTile(map, spriteID, gridX, gridY, EntityTypes::GATE), _linkId(linkId), _maxOpenAmount(
				clamp(openAmount, 0.0f, 1.0f)), _protrusion(_maxOpenAmount), _targetProtrusion(_maxOpenAmount),
				_openRequested(false)
{
	setAnimationType(Animations::ANIMATION_IDLE);
}

Gate::~Gate ()
{
}

uint8_t Gate::getProtrusionByte () const
{
	const float t = _maxOpenAmount > 0.0f ? (_protrusion / _maxOpenAmount) : 0.0f;
	return static_cast<uint8_t>(clamp(t, 0.0f, 1.0f) * 255.0f + 0.5f);
}

void Gate::setOpen (bool open)
{
	_openRequested = open;
	const float newTarget = open ? 0.0f : _maxOpenAmount;
	// One-shot stone scrape when motion actually starts or reverses.
	if (!fequals(newTarget, _targetProtrusion) && !fequals(_protrusion, newTarget))
		_map.sendSound(getVisMask(), SoundTypes::SOUND_GATE_MOVE, getPos());
	_targetProtrusion = newTarget;
}

void Gate::createBody ()
{
	PhysicsFixtureDef fd;
	fd.shapeType = PhysicsShapeType::Polygon;
	fd.useBox = true;
	fd.boxHalfWidth = _size.x / 2.0f * std::max(_protrusion, GATE_OPEN_THRESHOLD);
	fd.boxHalfHeight = _size.y / 2.0f;
	fd.density = DENSITY_STONE;
	fd.friction = 0.2f;
	fd.restitution = 0.0f;

	PhysicsBodyDef bd;
	bd.position.set(_pos.x, _pos.y);
	bd.type = PhysicsBodyType::Static;
	bd.fixedRotation = true;
	bd.angle = DegreesToRadians(_angle);

	_map.addToWorld(fd, bd, this);
	setGridDimensions(_gridWidth, _gridHeight, _angle);
	applyCollisionState();
}

void Gate::applyCollisionState ()
{
	if (_bodies.empty())
		return;
	const bool enabled = _protrusion >= GATE_OPEN_THRESHOLD;
	_bodies[0].setEnabled(enabled);
}

void Gate::sendState () const
{
	GameEvent.sendGateState(getVisMask(), getID(), getProtrusionByte());
}

void Gate::update (uint32_t deltaTime)
{
	MapTile::update(deltaTime);
	const float dt = deltaTime / 1000.0f;
	const float prev = _protrusion;
	if (_protrusion < _targetProtrusion) {
		_protrusion = std::min(_targetProtrusion, _protrusion + GATE_PROTRUSION_SPEED * dt);
	} else if (_protrusion > _targetProtrusion) {
		_protrusion = std::max(_targetProtrusion, _protrusion - GATE_PROTRUSION_SPEED * dt);
	}
	if (!fequals(prev, _protrusion)) {
		applyCollisionState();
		sendState();
		if (_protrusion <= GATE_OPEN_THRESHOLD)
			setAnimationType(Animations::ANIMATION_ACTIVE);
		else
			setAnimationType(Animations::ANIMATION_IDLE);
	}
}

bool Gate::shouldCollide (const IEntity* entity) const
{
	if (_protrusion < GATE_OPEN_THRESHOLD)
		return false;
	return entity->isSolid() || entity->isDynamic();
}

}
