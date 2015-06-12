#include "NPCFish.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/shared/constants/Density.h"
#include "caveexpress/server/map/Map.h"

namespace caveexpress {

NPCFish::NPCFish (Map& map, double magnitude, double amplitude) :
		NPCAggressive(EntityTypes::NPC_FISH, map), _magnitude(magnitude), _amplitude(amplitude)
{
	_currentSwimmingSpeed = b2Vec2_zero;
	_initialSwimmingSpeed = 2.0f;
	_spriteAlignment = ENTITY_ALIGN_MIDDLE_CENTER;
}

NPCFish::~NPCFish ()
{
}

void NPCFish::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	NPCAggressive::onPreSolve(contact, entity, oldManifold);
	// we hit a player - so the player is crashing
	if (!isDying() && entity->isPlayer()) {
		Player* player = static_cast<Player*>(entity);
		player->setCrashed(CRASH_NPC_FISH);
	}
	contact->SetEnabled(false);
}

void NPCFish::update (uint32_t deltaTime)
{
	NPCAggressive::update(deltaTime);
	// TODO: look for players and try to attack them
	const b2Vec2& position = getPos();
	// 1/4 * sin(x/4) => d/dx => 1/4 * cos(x/4) * (1 * 1/4 * x^0) => 1/16 * cos(x/4)
	const double divAmplitude = 1.0 / _amplitude;
	const double elevation = _magnitude * cos(position.x / divAmplitude) * _amplitude;
	const double radians = atan(elevation);
	const b2Vec2 pos(position.x, std::max(_map.getWaterHeight(), (float)(position.y + _magnitude * sinf(position.x) * _amplitude)));
	setPosAndAngle(pos, radians);

	if (!isDying())
		return;

	b2Vec2 v = getLinearVelocity();
	v.y = _map.getGravity() / 2.0f;
	setLinearVelocity(v);
}

const Animation& NPCFish::getFallingAnimation () const
{
	return getIdleAnimation();
}

void NPCFish::onSpawn ()
{
	if (getAnimationType() != Animation::NONE)
		return;

	if (getPos().x <= 0.1) {
		setSwimmingAnimation(Animations::ANIMATION_SWIMMING_RIGHT);
		_lastDirectionRight = true;
	} else {
		setSwimmingAnimation(Animations::ANIMATION_SWIMMING_LEFT);
		_lastDirectionRight = false;
	}
	setLinearVelocity(_currentSwimmingSpeed);
	_map.sendSpawnInfo(getPos(), _type);
}

float NPCFish::getDensity () const
{
	return DENSITY_NPC_FISH;
}

void NPCFish::setSwimmingAnimation (const Animation& animation)
{
	setState(NPCState::NPC_SWIMMING);
	setAnimationType(animation);
	float vx;
	if (animation == Animations::ANIMATION_SWIMMING_LEFT)
		vx = -_initialSwimmingSpeed;
	else
		vx = _initialSwimmingSpeed;
	_currentSwimmingSpeed = b2Vec2(vx, 0);
}

bool NPCFish::shouldCollide (const IEntity* entity) const
{
	if (isDying())
		return false;

	if (entity->isWater())
		return true;

	if (!NPCAggressive::shouldCollide(entity))
		return false;

	// flying npcs do only collide with players
	if (entity->isPlayer()) {
		const Player *p = static_cast<const Player*>(entity);
		return !p->isCrashed();
	}
	return false;
}

bool NPCFish::isRemove () const
{
	return _remove || NPCAggressive::isRemove();
}

}
