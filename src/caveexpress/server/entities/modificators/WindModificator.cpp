#include "WindModificator.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/Direction.h"
#include "caveexpress/server/map/Map.h"
#include "common/Shared.h"
#include <SDL_stdinc.h>

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

inline void WindModificator::getRelativePosition (b2Vec2& out) const
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

void WindModificator::setRelativePositionTo (const b2Vec2& pos)
{
	b2Vec2 modPos = pos;
	getRelativePosition(modPos);
	setPos(modPos);
}

void WindModificator::createBody (const b2Vec2 &pos, float shift)
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

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	b2Vec2 modPos = pos;
	getRelativePosition(modPos);
	bodyDef.position.Set(modPos.x, modPos.y);
	bodyDef.angle = DegreesToRadians(angle);
	bodyDef.fixedRotation = true;

	// counterclock wise - starting at upper left
	b2Vec2 vertices[4];
	const float x1 = 0.5f;
	const float x2 = x1 / _beginSizeDivisor;
	vertices[0].Set(0.0f, -x2);
	vertices[1].Set(0.0f, x2);
	vertices[2].Set(_modificatorSize, x1);
	vertices[3].Set(_modificatorSize, -x1);

	b2PolygonShape shape;
	shape.Set(vertices, SDL_arraysize(vertices));

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = DENSITY_AIR;

	b2Body* body = _map.getWorld()->CreateBody(&bodyDef);
	body->SetUserData((void*) this);
	body->CreateFixture(&fixtureDef);
	addBody(body);
}

void WindModificator::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	IEntity::onPreSolve(contact, entity, oldManifold);
	contact->SetEnabled(false);
}

bool WindModificator::shouldCollide (const IEntity* entity) const
{
	return entity->isPlayer() || entity->isPackage() || entity->isStone() || entity->isFruit();
}

void WindModificator::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);

	b2Body *ownBody = getBodies()[0];
	if (!_state) {
		ownBody->SetActive(false);
		return;
	} else {
		ownBody->SetActive(true);
	}

	for (b2ContactEdge* c = ownBody->GetContactList(); c != nullptr; c = c->next) {
		b2Contact *contact = c->contact;
		if (!contact->IsTouching())
			continue;

		b2Body *bodyA = contact->GetFixtureA()->GetBody();
		b2Body *bodyB = contact->GetFixtureB()->GetBody();
		b2Body *body = ownBody != bodyA ? bodyA : bodyB;

		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		const b2Manifold *manifold = contact->GetManifold();
		for (int i = 0; i < manifold->pointCount; ++i) {
			const b2Vec2& contactPoint = worldManifold.points[i];
			applyImpulse(body, contactPoint, _force);
		}
	}
}

void WindModificator::applyImpulse (b2Body* body, b2Vec2 contactPoint, float force) const
{
	const b2Vec2 direction = body->GetPosition() - getPos();
	const float distance = 0.1f * b2Distance(body->GetPosition(), getPos());
	const float impulseMag = std::min(70.0f, force * _modificatorSize / distance);
	body->ApplyLinearImpulse(impulseMag * direction, contactPoint, true);
}
