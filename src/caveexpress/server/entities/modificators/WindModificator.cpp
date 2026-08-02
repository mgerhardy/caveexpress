#include "WindModificator.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/Direction.h"
#include "caveexpress/server/map/Map.h"
#include "common/Shared.h"
#include <SDL_stdinc.h>

namespace caveexpress {

WindModificator::WindModificator (Map& map, Direction direction, float force, float size, float beginSizeDivisior) :
		IWorldModificator(map), _state(false), _direction(direction), _shift(
				0.0f), _force(force), _modificatorSize(size), _beginSizeDivisor(
				beginSizeDivisior) {
}

WindModificator::~WindModificator ()
{
}

void WindModificator::setModificatorState (bool enable)
{
	_state = enable;
}

inline void WindModificator::getRelativePosition (PhysicsVec2& out) const
{
	switch (_direction) {
	case DIRECTION_RIGHT:
		out.x += _shift;
		break;
	case DIRECTION_LEFT:
		out.x -= _shift;
		break;
	case DIRECTION_UP:
		out.y -= _shift;
		break;
	case DIRECTION_DOWN:
		out.y += _shift;
		break;
	default:
		System.exit("unknown direction given for wind modificator", 1);
	}
}

void WindModificator::setRelativePositionTo (const PhysicsVec2& pos)
{
	PhysicsVec2 modPos = pos;
	getRelativePosition(modPos);
	setPos(modPos);
}

void WindModificator::createBody (const PhysicsVec2 &pos, float shift)
{
	if (!_bodies.empty())
		return;

	_shift = shift;

	float angle;
	switch (_direction) {
	case DIRECTION_UP:
		angle = 270.0f;
		break;
	case DIRECTION_LEFT:
		angle = 180.0f;
		break;
	case DIRECTION_DOWN:
		angle = 90.0f;
		break;
	default:
		angle = 0.0f;
		break;
	}

	PhysicsBodyDef bodyDef;
	bodyDef.userData = (uintptr_t)this;
	bodyDef.type = PhysicsBodyType::Static;
	PhysicsVec2 modPos = pos;
	getRelativePosition(modPos);
	bodyDef.position.set(modPos.x, modPos.y);
	bodyDef.angle = DegreesToRadians(angle);
	bodyDef.fixedRotation = true;

	// counterclock wise - starting at upper left
	PhysicsVec2 vertices[4];
	const float x1 = 0.5f;
	const float x2 = x1 / _beginSizeDivisor;
	vertices[0].set(0.0f, -x2);
	vertices[1].set(0.0f, x2);
	vertices[2].set(_modificatorSize, x1);
	vertices[3].set(_modificatorSize, -x1);

	PhysicsFixtureDef fixtureDef;
	fixtureDef.shapeType = PhysicsShapeType::Polygon;
	fixtureDef.vertexCount = 4;
	fixtureDef.vertices[0] = vertices[0];
	fixtureDef.vertices[1] = vertices[1];
	fixtureDef.vertices[2] = vertices[2];
	fixtureDef.vertices[3] = vertices[3];
	fixtureDef.density = DENSITY_AIR;

	PhysicsBody body = _map.getWorld()->createBody(bodyDef);
	body.createFixture(fixtureDef);
	addBody(body);
}

void WindModificator::onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold)
{
	IEntity::onPreSolve(contact, entity, oldManifold);
	contact.setEnabled(false);
}

bool WindModificator::shouldCollide (const IEntity* entity) const
{
	return entity->isPlayer() || entity->isPackage() || entity->isStone() || entity->isFruit();
}

void WindModificator::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);

	PhysicsBody ownBody = getBodies()[0];
	if (!_state) {
		ownBody.setEnabled(false);
		return;
	} else {
		ownBody.setEnabled(true);
	}

	for (PhysicsContactEdge c = ownBody.getContactList(); c.isValid(); c = c.next()) {
		PhysicsContact contact = c.contact;
		if (!contact.isTouching())
			continue;

		PhysicsBody bodyA = contact.getFixtureA().getBody();
		PhysicsBody bodyB = contact.getFixtureB().getBody();
		PhysicsBody body = ownBody != bodyA ? bodyA : bodyB;

		PhysicsWorldManifold worldManifold;
		contact.getWorldManifold(worldManifold);

		const PhysicsManifold manifold = contact.getManifold();
		for (int i = 0; i < manifold.pointCount; ++i) {
			const PhysicsVec2& contactPoint = worldManifold.points[i];
			applyImpulse(body, contactPoint, _force);
		}
	}
}

void WindModificator::applyImpulse (PhysicsBody body, PhysicsVec2 contactPoint, float force) const
{
	const PhysicsVec2 direction = body.getPosition() - getPos();
	const float distance = 0.1f * physDistance(body.getPosition(), getPos());
	const float impulseMag = std::min(70.0f, force * _modificatorSize / distance);
	body.applyLinearImpulse(impulseMag * direction, contactPoint, true);
}

}
