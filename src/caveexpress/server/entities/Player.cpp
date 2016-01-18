#include "Player.h"
#include "caveexpress/server/entities/Platform.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/Stone.h"
#include "caveexpress/server/entities/Bomb.h"
#include "caveexpress/server/entities/Package.h"
#include "caveexpress/server/entities/npcs/NPCFriendly.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/shared/CaveExpressAchievement.h"
#include "common/Log.h"
#include "network/INetwork.h"
#include "common/System.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/ConfigManager.h"
#include "network/IProtocolHandler.h"
#include "caveexpress/shared/CaveExpressSoundType.h"

namespace caveexpress {

namespace {
const float gravityScale = 0.2f;
}

Player::Player (Map& map, ClientId clientId) :
		IEntity(EntityTypes::PLAYER, map), _touching(nullptr), _hitpoints(
				0), _lives(0), _collectedNPC(nullptr), _acceleration(b2Vec2_zero), _fingerAcceleration(
				false), _accelerateX(0), _accelerateY(0), _clientId(clientId), _lastAccelerate(
				0), _name(""), _lastFruitCollected(0), _fruitsCollectedInARow(
				0), _revoluteJoint(nullptr), _crashReason(CRASH_NONE) {
	_godMode = Config.getConfigVar("godmode");
	_maxHitPoints = Config.getConfigVar("maxhitpoints");
	_hitpoints = _maxHitPoints->getIntValue();
	_fruitHitPoints = Config.getConfigVar("fruithitpoints");
	_damageThreshold = Config.getConfigVar("damagethreshold");
	_amountOfFruitsForANewLife = Config.getConfigVar("amountoffruitsforanewlife");
	_fruitCollectDelayForANewLife = Config.getConfigVar("fruitcollectdelayforanewlife");
	setAnimationType(Animations::ANIMATION_IDLE);
	setState(PlayerState::PLAYER_IDLE);
	memset(_collectedEntities, 0, sizeof(_collectedEntities));
}

Player::~Player ()
{
}

void Player::accelerate (Direction dir)
{
	if (isCrashed())
		return;

	_lastAccelerate = _time;
	b2Vec2 v(0.0f, 0.0f);
	const float gravity = getGravity().y;
	const float scaledGravity = gravity * getGravityScale();

	if (dir & DIRECTION_UP) {
		if (_acceleration.y >= 0.0f) {
			v.Set(0.0f, -gravity);
		}
	} else if (dir & DIRECTION_DOWN) {
		v.Set(0.0f, gravity / 2.0);
	}

	if (dir & DIRECTION_LEFT) {
		if (_acceleration.x >= 0.0f) {
			v.Set(-scaledGravity, 0.0f);
		}
	} else if (dir & DIRECTION_RIGHT) {
		if (_acceleration.x <= 0.0f) {
			v.Set(scaledGravity, 0.0f);
		}
	}

	setAnimationType(Animations::ANIMATION_FLYING);
	_acceleration += v;

	const float maxHorizontalVelocity = 1.7f;
	_acceleration.x = clamp(_acceleration.x, -maxHorizontalVelocity, maxHorizontalVelocity);
	const float maxVerticalVelocity = gravity;
	_acceleration.y = clamp(_acceleration.y, -maxVerticalVelocity, maxVerticalVelocity);
}

void Player::resetAcceleration ()
{
	if (isCrashed())
		return;

	_fingerAcceleration = false;
	setAnimationType(Animations::ANIMATION_IDLE);
}

void Player::setAcceleration (int dx, int dy)
{
	if (isCrashed())
		return;

	_fingerAcceleration = true;
	setAnimationType(Animations::ANIMATION_FLYING);
	_accelerateX = dx;
	_accelerateY = dy;
}

void Player::subtractHitpoints (uint16_t hitpoints)
{
	const int oldHitpoints = _hitpoints;
	const int maxHitpoints = _maxHitPoints->getIntValue();
	_hitpoints = clamp(_hitpoints - hitpoints, 0, maxHitpoints);
	if (oldHitpoints != _hitpoints) {
		GameEvent.updateHitpoints(*this);
		_map.sendSound(ClientIdToClientMask(getClientId()), SoundTypes::SOUND_PLAYER_PAIN, getPos());
	}
}

void Player::addHitpoints (uint16_t hitpoints)
{
	const int oldHitpoints = _hitpoints;
	const int maxHitpoints = _maxHitPoints->getIntValue();
	_hitpoints = clamp(oldHitpoints + hitpoints, 0, maxHitpoints);
	if (oldHitpoints != _hitpoints) {
		GameEvent.updateHitpoints(*this);
	}
}

bool Player::shouldApplyWind () const
{
	return true;
}

inline float Player::getCompleteMass () const
{
	float mass = getMass() * getGravityScale();
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		const Collected &c = _collectedEntities[i];
		const EntityType *entityType = c.entityType;
		if (entityType == nullptr || !EntityTypes::isPackage(*entityType))
			continue;
		const Package *package = static_cast<const Package*>(c.entity);
		mass += package->getMass() * package->getGravityScale();
	}
	return mass;
}

void Player::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);

	if (isCrashed()) {
		// before we crash, we should drop the stuff we are carrying
		drop();
	}

	if (System.hasTouch() && _fingerAcceleration) {
		const float mass = getCompleteMass();
		const b2Vec2& gravity = getGravity();
		b2Vec2 v = mass * gravity;
		const int delta = 1;
		if (_accelerateY <= -delta) {
			// go upwards
			v.y *= _accelerateY;
		} else if (_accelerateY >= delta) {
			// go downwards
			v.y *= 0.5f;
		} else {
			// stay in the air (see below)
			v.y *= 0.0f;
		}

		const float horizontalMoveSpeed = 1.0f;
		if (std::abs(_accelerateX) >= delta)
			v.x = horizontalMoveSpeed * _accelerateX;

		const float maxHorizontalVelocity = gravity.y;
		v.x = clamp(v.x, -maxHorizontalVelocity, maxHorizontalVelocity);
		v.y = clamp(v.y, -gravity.y * 3.0f, gravity.y);

		Log::debug(LOG_GAMEIMPL, "v(%f:%f), accel(%i:%i)", v.x, v.y, _accelerateX, _accelerateY);

		if (fabs(v.y) < 0.0001f) {
			const b2Vec2 force = -mass * getGravity();
			Log::debug(LOG_GAMEIMPL, "f: (%f:%f)", force.x, force.y);
			applyForce(force);
		}
		applyLinearImpulse(v);
	} else {
		const float maxSpeed = 8.0f;
		const b2Vec2 force = getMass() * _acceleration;
		b2Vec2 velocity = getLinearVelocity();
		const float32 speed = velocity.Normalize();
		const b2Vec2 cappedV = std::min(speed, maxSpeed) * velocity;
		_bodies[0]->SetLinearVelocity(cappedV);
		applyForce(force);
	}

	const float angle = getAngle();
	_revoluteJoint->SetMotorSpeed(-angle);

	if (_hitpoints <= 0)
		setCrashed(CRASH_DAMAGE);

	const EntityType *arrived = nullptr;
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		Collected &c = _collectedEntities[i];
		const EntityType *entityType = c.entityType;
		if (entityType == nullptr || !EntityTypes::isPackage(*entityType))
			continue;
		const Package *package = static_cast<const Package*>(c.entity);
		if (package->isArrived() || package->isDestroyed() || package->isDelivered()) {
			// 'drop' it
			memset(&c, 0, sizeof(c));
			arrived = &package->getType();
		}
	}

	// nothing was dropped
	if (arrived == nullptr)
		return;

	// check whether we still carry something of the same type
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		Collected &c = _collectedEntities[i];
		const EntityType *entityType = c.entityType;
		if (entityType == nullptr || *entityType != *arrived)
			continue;
		return;
	}
	// no, we don't - inform the client about this
	GameEvent.sendCollectState(_clientId, *arrived, false);
}

void Player::setCrashed (const PlayerCrashReason& reason)
{
#ifdef DEBUG
	if (_godMode->getBoolValue())
		return;
#endif
	if (_map.isDone())
		return;

	if (isCrashed())
		return;

	if (Config.isModeHard()) {
		reduceLive();
		GameEvent.updateLives(*this);
	}

	setState(PlayerState::PLAYER_CRASHED);
	setAnimationType(Animations::ANIMATION_CRASHED);
	_crashReason = reason;

	const float rumbleLengthMillis = 500.0f;
	const SoundType* sound;
	switch (reason) {
	case CRASH_NPC_WALKING:
	case CRASH_NPC_MAMMUT:
		sound = &SoundTypes::SOUND_PLAYER_CRASH_NPC_WALKING;
		GameEvent.sendRumble(2.0f, rumbleLengthMillis);
		break;
	case CRASH_NPC_FISH:
		sound = &SoundTypes::SOUND_PLAYER_CRASH_NPC_FISH;
		GameEvent.sendRumble(0.5f, rumbleLengthMillis);
		break;
	case CRASH_NPC_FLYING:
		sound = &SoundTypes::SOUND_PLAYER_CRASH_NPC_FLYING;
		GameEvent.sendRumble(1.0f, rumbleLengthMillis);
		break;
	case CRASH_DAMAGE:
		sound = &SoundTypes::SOUND_PLAYER_CRASH_HITPOINTS;
		break;
	default:
		sound = nullptr;
		break;
	}

	if (sound)
		_map.sendSound(getVisMask(), *sound, getPos());
}

void Player::resetAcceleration (Direction dir)
{
	if (isCrashed())
		return;

	if (dir != 0) {
		if (dir & DIRECTION_HORIZONTAL)
			_acceleration.x = 0.0f;
		if (dir & DIRECTION_VERTICAL)
			_acceleration.y = 0.0f;
	} else {
		_acceleration = b2Vec2_zero;
	}
	if (b2Vec2Equals(_acceleration, b2Vec2_zero))
		setAnimationType(Animations::ANIMATION_IDLE);
}

void Player::applyForce (const b2Vec2& v)
{
	_bodies[0]->ApplyForceToCenter(v, true);
}

bool Player::shouldCollide (const IEntity* entity) const
{
	if (entity->isPackage() && getPos().y < entity->getPos().y)
		return false;
	if (entity->isPlayer()) {
		const Player* player = static_cast<const Player*>(entity);
		return !player->isCrashed();
	}
	return entity->isSolid() || entity->isWater();
}

void Player::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	if (isCrashed())
		return;
	if (!entity->isSolid() && !entity->isPlatform())
		return;

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);
	b2PointState state1[2], state2[2];
	b2GetPointStates(state1, state2, oldManifold, contact->GetManifold());
	if (state2[0] != b2_addState)
		return;

	const b2Body* bodyA = contact->GetFixtureA()->GetBody();
	const b2Body* bodyB = contact->GetFixtureB()->GetBody();
	const b2Vec2& point = worldManifold.points[0];
	const b2Vec2& vA = bodyA->GetLinearVelocityFromWorldPoint(point);
	const b2Vec2& vB = bodyB->GetLinearVelocityFromWorldPoint(point);
	const float approachVelocity = fabs(b2Dot(vB - vA, worldManifold.normal));
	const float damageThreshold = _damageThreshold->getFloatValue();
	if (approachVelocity <= damageThreshold)
		return;

	const float factor = approachVelocity - damageThreshold;
	const int maxHitpoints = _maxHitPoints->getIntValue();
	const int hitpointReduceAmount = std::max(1, (int)(maxHitpoints / 10 * (1.0f + factor)));
	subtractHitpoints(hitpointReduceAmount);
	Log::info(LOG_GAMEIMPL, "damageThreshold: %f, approachVelocity: %f, factor: %f, hitpointReduceAmount: %i",
			   damageThreshold, approachVelocity, factor, hitpointReduceAmount);
	GameEvent.sendRumble(factor, 500);
}

void Player::onDeath ()
{
	// TODO: use UI_WINDOW_GAMEOVER
	GameEvent.backToMain("gameover");
}

bool Player::canCarry (const IEntity* entity) const
{
	if (!entity->isPackage())
		return isFree();

	// if you have only packages collected, then you can collect as many as you want
	int free = 0;
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		const EntityType *entityType = _collectedEntities[i].entityType;
		if (entityType == nullptr) {
			++free;
		} else if (!EntityTypes::isPackage(*entityType)) {
			if (EntityTypes::isStone(*entityType))
				_map.sendMessage(_clientId, "Drop the stone before collecting the package");
			else
				_map.sendMessage(_clientId, "You currently can't collect the package");
			return false;
		}
	}
	return free > 0;
}

bool Player::collect (CollectableEntity* entity)
{
	if (isCrashed())
		return false;

	const EntityType & entityType = entity->getType();
	if (EntityTypes::isFruit(entityType) || EntityTypes::isEgg(entityType)) {
		const uint32_t fruitCollectDelayMillis = _fruitCollectDelayForANewLife->getIntValue();
		if (_lastFruitCollected == 0 || _time - _lastFruitCollected < fruitCollectDelayMillis) {
			_lastFruitCollected = _time;
			++_fruitsCollectedInARow;
			if (++_fruitsCollectedInARow == _amountOfFruitsForANewLife->getIntValue()) {
				_fruitsCollectedInARow = 0;
				_lastFruitCollected = 0;
				if (Config.isModeEasy())
					addLife();
			}
		} else {
			_fruitsCollectedInARow = 0;
			_lastFruitCollected = 0;
		}
		Achievements::PICK_UP_FRUIT.unlock();
		Achievements::COLLECT_100_FRUITS.unlock();
		_map.sendSound(ClientIdToClientMask(getClientId()), SoundTypes::SOUND_FRUIT_COLLECTED);
		addHitpoints(_fruitHitPoints->getIntValue());
		return true;
	}

	if (!canCarry(entity))
		return false;

	Log::info(LOG_GAMEIMPL, "collected entity of type: %s", entityType.name.c_str());
	const Collected c = { &entityType, entity };
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		if (_collectedEntities[i].entityType != nullptr)
			continue;

		_collectedEntities[i] = c;
		break;
	}
	if (EntityTypes::isStone(entityType)) {
		Achievements::COLLECT_10_STONES.unlock();
		Achievements::COLLECT_100_STONES.unlock();
	}

	GameEvent.sendCollectState(_clientId, entityType, true);
	return true;
}

void Player::drop ()
{
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		const Collected &c = _collectedEntities[i];
		const EntityType *entityType = c.entityType;
		if (entityType == nullptr)
			continue;
		if (EntityTypes::isStone(*entityType)) {
			Stone *entity = new Stone(_map, getPos().x, getPos().y, this);
			entity->createBody();
		} else if (EntityTypes::isBomb(*entityType)) {
			Bomb *entity = new Bomb(_map, getPos().x, getPos().y, this);
			entity->createBody();
			entity->initiateDetonation();
		} else if (EntityTypes::isPackage(*entityType)) {
			Package *entity = static_cast<Package*>(c.entity);
			entity->removeRopeJoint();
			entity->setCollected(false, this);
		} else {
			Log::error(LOG_GAMEIMPL, "unknown entity type: %s", entityType->name.c_str());
			continue;
		}
		Log::info(LOG_GAMEIMPL, "drop entity of type: %s", entityType->name.c_str());
		GameEvent.sendCollectState(_clientId, *entityType, false);
	}

	memset(_collectedEntities, 0, sizeof(_collectedEntities));
}

void Player::createBody (const b2Vec2 &pos)
{
	b2World *world = _map.getWorld();
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x, pos.y - 0.2f);
	bodyDef.fixedRotation = true;
	b2Body* center = world->CreateBody(&bodyDef);
	b2CircleShape centerShape;
	centerShape.m_radius = 0.01f;
	// Define the dynamic body fixture.
	b2FixtureDef centerFixtureDef;
	centerFixtureDef.isSensor = true;
	centerFixtureDef.shape = &centerShape;
	center->CreateFixture(&centerFixtureDef);
	center->SetGravityScale(gravityScale);

	bodyDef.fixedRotation = false;
	b2Body* body = world->CreateBody(&bodyDef);

	const float hx = _size.x / 2.0f;
	const float hy = _size.y / 2.0f;
	b2Vec2 vertices[4];

	vertices[0].Set(-hx, -hy);
	vertices[1].Set( hx, -hy);
	vertices[2].Set( hx / 2.0f,  hy);
	vertices[3].Set(-hx / 2.0f,  hy);

	b2PolygonShape shape;
	shape.Set(vertices, SDL_arraysize(vertices));

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;

	// Set the box density to be non-zero, so it will be dynamic.
	fixtureDef.density = DENSITY_PLAYER;

	// Override the default friction.
	fixtureDef.friction = 1.0f;

	body->SetUserData(this);

	// Add the shape to the body.
	body->CreateFixture(&fixtureDef);
	body->SetGravityScale(gravityScale);

	// the order matters
	addBody(body);
	addBody(center);

	b2RevoluteJointDef revoluteJointDef;
	revoluteJointDef.Initialize(center, body, pos);
	revoluteJointDef.lowerAngle = DegreesToRadians(-10);
	revoluteJointDef.upperAngle = DegreesToRadians(10);
	revoluteJointDef.enableLimit = true;
	revoluteJointDef.collideConnected = false;
	revoluteJointDef.enableMotor = true;
	revoluteJointDef.maxMotorTorque = 20;
	revoluteJointDef.motorSpeed = 0.0f;
	revoluteJointDef.maxMotorTorque = 100.0f;
	_revoluteJoint = static_cast<b2RevoluteJoint*>(world->CreateJoint(&revoluteJointDef));
}

bool Player::isCloseOverSolid (float distance) const
{
	b2Vec2 end = getPos();
	end.y += distance;
	IEntity* entity = nullptr;
	_map.rayTrace(getPos(), end, &entity);
	if (entity != nullptr && entity->isSolid())
		return true;

	return false;
}

bool Player::isLanded () const
{
	if (!b2Vec2Equals(getLinearVelocity(), b2Vec2_zero))
		return false;

	if (isCloseOverSolid())
		return true;

	return false;
}

void Player::setPlatform (Platform* entity)
{
	if (_touching == entity)
		return;

	_touching = entity;
	if (!_touching)
		return;

	_map.sendSound(getVisMask(), SoundTypes::SOUND_PLAYER_LAND, getPos());

	CaveMapTile *cave = _touching->getCave();
	if (cave == nullptr)
		return;
	INPCCave *npc = cave->getNPC();
	if (npc == nullptr)
		return;
	if (npc->isDeliverPackage())
		return;

	npc->resetTriggerMovement();
}

void Player::setCollectedNPC(NPCFriendly *npc) {
	// we can't collect a npc if we have collected something else
	if (npc && !isFree())
		return;

	_collectedNPC = npc;
	if (npc != nullptr) {
		npc->setCollected();
		GameEvent.sendTargetCave(ClientIdToClientMask(_clientId), npc->getTargetCaveNumber());
	} else {
		GameEvent.sendTargetCave(ClientIdToClientMask(_clientId), 0);
	}
}

}
