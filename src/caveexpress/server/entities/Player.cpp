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
#include "common/Math.h"
#include "network/INetwork.h"
#include "common/System.h"
#include "caveexpress/shared/constants/Density.h"
#include "common/ConfigManager.h"
#include "network/IProtocolHandler.h"
#include "caveexpress/shared/CaveExpressCooldown.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "caveexpress/shared/constants/ConfigVars.h"
#include <cstring>

namespace caveexpress {

namespace {
const float gravityScale = 0.3f;
}

Player::Player (Map& map, ClientId clientId) :
		IEntity(EntityTypes::PLAYER, map), _touching(nullptr), _invulnerableTime(0u), _powerUpTime(0u), _collectedNPC(nullptr), _acceleration(PhysicsVec2_zero), _fingerAcceleration(
				false), _accelerateX(0), _accelerateY(0), _clientId(clientId), _lastAccelerate(0), _name(""), _lastFruitCollected(0), _hitpoints(
				0), _lives(0), _fruitsCollectedInARow(0), _revoluteJoint(), _crashReason(CRASH_NONE) {
	_godMode = Config.getConfigVar(GOD_MODE);
	_maxHitPoints = Config.getConfigVar(MAX_HITPOINTS);
	_hitpoints = _maxHitPoints->getIntValue();
	_fruitHitPoints = Config.getConfigVar(FRUIT_HITPOINTS);
	_damageThreshold = Config.getConfigVar(DAMAGE_THRESHOLD);
	_amountOfFruitsForANewLife = Config.getConfigVar(AMOUNT_OF_FRUITS_FOR_A_NEW_LIFE);
	_fruitCollectDelayForANewLife = Config.getConfigVar(FRUIT_COLLECT_DELAY_FOR_A_NEW_LIFE);
	setAnimationType(Animations::ANIMATION_IDLE);
	setState(PlayerState::PLAYER_IDLE);
	memset(_collectedEntities, 0, sizeof(_collectedEntities));
}

Player::~Player ()
{
}

// see MovementMessage and MovementHandler
void Player::accelerate (Direction dir)
{
	if (isCrashed())
		return;

	_lastAccelerate = _time;
	PhysicsVec2 v(0.0f, 0.0f);
	const float gravity = getGravity().y;
	const float scaledGravity = gravity * getGravityScale();

	if (dir & DIRECTION_UP) {
		if (_acceleration.y >= 0.0f) {
			v.set(0.0f, -gravity);
		}
	} else if (dir & DIRECTION_DOWN) {
		v.set(0.0f, gravity / 2.0f);
	}

	if (dir & DIRECTION_LEFT) {
		if (_acceleration.x >= 0.0f) {
			v.set(-scaledGravity, 0.0f);
		}
	} else if (dir & DIRECTION_RIGHT) {
		if (_acceleration.x <= 0.0f) {
			v.set(scaledGravity, 0.0f);
		}
	}

	setAnimationType(Animations::ANIMATION_FLYING);
	_acceleration += v;

	const float maxHorizontalVelocity = 1.7f;
	_acceleration.x = clamp(_acceleration.x, -maxHorizontalVelocity, maxHorizontalVelocity);
	const float maxVerticalVelocity = gravity;
	_acceleration.y = clamp(_acceleration.y, -maxVerticalVelocity, maxVerticalVelocity);
}

void Player::resetFingerAcceleration ()
{
	if (isCrashed())
		return;

	_fingerAcceleration = false;
	setAnimationType(Animations::ANIMATION_IDLE);
}

void Player::setFingerAcceleration (int dx, int dy)
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
#ifdef DEBUG
	if (_godMode->getBoolValue())
		return;
#endif
	if (_time <= _invulnerableTime)
		return;

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
		const Package *package = assert_cast<const Package*, const CollectableEntity*>(c.entity);
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

	int packages = 0;
	// in power up mode the carried masses are not taken into account.
	if (_time <= _powerUpTime) {
		for (int i = 0; i < MAX_COLLECTED; ++i) {
			const Collected &c = _collectedEntities[i];
			const EntityType *entityType = c.entityType;
			if (entityType == nullptr || !EntityTypes::isPackage(*entityType))
				continue;
			++packages;
		}
	}
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		const Collected &c = _collectedEntities[i];
		const EntityType *entityType = c.entityType;
		if (entityType == nullptr || !EntityTypes::isPackage(*entityType))
			continue;
		float scale = 1.0f;
		if (packages > 1) {
			scale /= ((float)packages - 1.0f);
		}
		c.entity->setGravityScale(scale);
	}

	if (_fingerAcceleration) {
		const float mass = getCompleteMass();
		const PhysicsVec2& gravity = getGravity();
		PhysicsVec2 v = mass * gravity;
		const int delta = 1;
		if (_accelerateY <= -delta) {
			// go upwards
			v.y *= (float)_accelerateY;
		} else if (_accelerateY >= delta) {
			// go downwards
			v.y *= 0.5f;
		} else {
			// stay in the air (see below)
			v.y *= 0.0f;
		}

		const float horizontalMoveSpeed = 1.0f;
		if (std::abs(_accelerateX) >= delta)
			v.x = horizontalMoveSpeed * (float)_accelerateX;

		const float maxHorizontalVelocity = gravity.y;
		v.x = clamp(v.x, -maxHorizontalVelocity, maxHorizontalVelocity);
		v.y = clamp(v.y, -gravity.y * 3.0f, gravity.y);

		Log::debug(LOG_GAMEIMPL, "v(%f:%f), accel(%i:%i)", v.x, v.y, _accelerateX, _accelerateY);

		if (fabs(v.y) < 0.0001f) {
			const PhysicsVec2 force = -mass * getGravity();
			Log::debug(LOG_GAMEIMPL, "f: (%f:%f)", force.x, force.y);
			applyForce(force);
		}
		applyLinearImpulse(v);
	} else {
		const float maxSpeed = 8.0f;
		PhysicsVec2 force = getMass() * _acceleration;
		force.x *= _map.getFlyingSpeedX();

		PhysicsVec2 velocity = getLinearVelocity();
		const float speed = velocity.normalize();
		const PhysicsVec2 cappedV = std::min(speed, maxSpeed) * velocity;
		_bodies[0].setLinearVelocity(cappedV);
		applyForce(force);
	}

	const float angle = getAngle();
	_revoluteJoint.setMotorSpeed(-angle);

	if (_hitpoints <= 0)
		setCrashed(CRASH_DAMAGE);

	const EntityType *arrived = nullptr;
	for (int i = 0; i < MAX_COLLECTED; ++i) {
		Collected &c = _collectedEntities[i];
		const EntityType *entityType = c.entityType;
		if (entityType == nullptr || !EntityTypes::isPackage(*entityType))
			continue;
		const Package *package = assert_cast<const Package*, const CollectableEntity*>(c.entity);
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
	if (_time <= _invulnerableTime)
		return;

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

	const int rumbleLengthMillis = 500;
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
	case CRASH_LAVA:
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
		_acceleration = PhysicsVec2_zero;
	}
	if (physVec2Equals(_acceleration, PhysicsVec2_zero))
		setAnimationType(Animations::ANIMATION_IDLE);
}

void Player::applyForce (const PhysicsVec2& v)
{
	_bodies[0].applyForceToCenter(v, true);
}

bool Player::shouldCollide (const IEntity* entity) const
{
	if (entity->isPackage() && getPos().y < entity->getPos().y)
		return false;
	if (entity->isPlayer()) {
		const Player* player = assert_cast<const Player*, const IEntity*>(entity);
		return !player->isCrashed();
	}
	return entity->isSolid() || entity->isWater() || entity->isLava();
}

void Player::onContact (PhysicsContact contact, IEntity* entity)
{
	IEntity::onContact(contact, entity);
	if (entity == nullptr || !entity->isLava())
		return;
	if (isCrashed())
		return;

	// Only the "lava" fixture is lethal; the rock base ("solid") is standable.
	PhysicsFixture fixture;
	if (contact.getFixtureA().getBody().getUserData() == (uintptr_t)entity)
		fixture = contact.getFixtureA();
	else
		fixture = contact.getFixtureB();
	const char* userData = reinterpret_cast<const char*>(fixture.getUserData());
	if (userData != nullptr && userData[0] != '\0' && strcmp(userData, "lava") != 0)
		return;

	setCrashed(CRASH_LAVA);
}

void Player::onPreSolve (PhysicsContact contact, IEntity* entity, const PhysicsManifold& oldManifold)
{
	if (isCrashed())
		return;
	if (!entity->isSolid() && !entity->isPlatform())
		return;

	PhysicsPointState state1[2], state2[2];
	physGetPointStates(state1, state2, oldManifold, contact.getManifold());
	if (state2[0] != PhysicsPointState::Add)
		return;

	damageFromHit (contact, entity);
}

void Player::damageFromHit (PhysicsContact contact, IEntity* entity)
{
	PhysicsWorldManifold worldManifold;
	contact.getWorldManifold(worldManifold);
	const PhysicsBody bodyA = contact.getFixtureA().getBody();
	const PhysicsBody bodyB = contact.getFixtureB().getBody();
	const PhysicsVec2& point = worldManifold.points[0];
	const PhysicsVec2& vA = bodyA.getLinearVelocityFromWorldPoint(point);
	const PhysicsVec2& vB = bodyB.getLinearVelocityFromWorldPoint(point);
	const float approachVelocity = fabs(physDot(vB - vA, worldManifold.normal));
	const float damageThreshold = _damageThreshold->getFloatValue();
	if (approachVelocity <= damageThreshold)
		return;

	const float factor = approachVelocity - damageThreshold;
	const float maxHitpoints = _maxHitPoints->getFloatValue();
	const int hitpointReduceAmount = std::max(1, (int)(maxHitpoints / 10.0f * (1.0f + factor)));
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

	const EntityType &entityType = entity->getType();
	if (EntityTypes::isFruit(entityType)) {
		if (EntityTypes::isBanana(entityType)) {
			_powerUpTime = _time + Cooldowns::POWERUP.getRuntime();
			_map.sendCooldown(_clientId, Cooldowns::POWERUP);
		}
		const uint32_t fruitCollectDelayMillis = _fruitCollectDelayForANewLife->getIntValue();
		if (_lastFruitCollected == 0 || _time - _lastFruitCollected < fruitCollectDelayMillis) {
			_lastFruitCollected = _time;
			++_fruitsCollectedInARow;
			if (++_fruitsCollectedInARow == _amountOfFruitsForANewLife->getIntValue()) {
				_fruitsCollectedInARow = 0;
				_lastFruitCollected = 0;
				if (Config.isModeHard())
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
	} else if (EntityTypes::isEgg(entityType)) {
		_invulnerableTime = _time + Cooldowns::INVULNERABLE.getRuntime();
		_map.sendCooldown(_clientId, Cooldowns::INVULNERABLE);
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
		GameEvent.sendTargetCave(ClientIdToClientMask(_clientId), 100);
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
			GameEvent.sendTargetCave(ClientIdToClientMask(_clientId), 0);
		} else if (EntityTypes::isBomb(*entityType)) {
			Bomb *entity = new Bomb(_map, getPos().x, getPos().y, this);
			entity->createBody();
			entity->initiateDetonation();
		} else if (EntityTypes::isPackage(*entityType)) {
			Package *entity = assert_cast<Package*, CollectableEntity*>(c.entity);
			entity->removeRopeJoint();
			entity->setCollected(false, this);
			entity->setGravityScale(1.0f);
		} else {
			Log::error(LOG_GAMEIMPL, "unknown entity type: %s", entityType->name.c_str());
			continue;
		}
		Log::info(LOG_GAMEIMPL, "drop entity of type: %s", entityType->name.c_str());
		GameEvent.sendCollectState(_clientId, *entityType, false);
	}

	memset(_collectedEntities, 0, sizeof(_collectedEntities));
}

void Player::createBody (const PhysicsVec2 &pos)
{
	// this is creating a body with a non-rotateable circle as center,
	// an attached polygon that is limited in rotation angles - and
	// both bodies are connected by a revolute joint (which ensures
	// the rotation limit mentioned earlier)
	PhysicsWorld *world = _map.getWorld();

	// create the circle
	PhysicsBody circleBody;
	{
		PhysicsBodyDef circleBodyDef;
		circleBodyDef.type = PhysicsBodyType::Dynamic;
		circleBodyDef.position.set(pos.x, pos.y - 0.2f);
		circleBodyDef.fixedRotation = true;
		circleBodyDef.userData = (uintptr_t)this;
		circleBody = world->createBody(circleBodyDef);
		PhysicsFixtureDef centerFixtureDef;
		centerFixtureDef.shapeType = PhysicsShapeType::Circle;
		centerFixtureDef.radius = 0.09f;
		centerFixtureDef.isSensor = true;
		centerFixtureDef.density = DENSITY_PLAYER;
		circleBody.createFixture(centerFixtureDef);
		circleBody.setGravityScale(gravityScale);
	}

	// create the polygon body that should be limited in rotation
	PhysicsBody body;
	{
		PhysicsBodyDef polygonBodyDef;
		polygonBodyDef.type = PhysicsBodyType::Dynamic;
		polygonBodyDef.position.set(pos.x, pos.y - 0.2f);
		polygonBodyDef.fixedRotation = false;
		polygonBodyDef.userData = (uintptr_t)this;
		body = world->createBody(polygonBodyDef);
		const float hx = _size.x / 2.0f;
		const float hy = _size.y / 2.0f;

		PhysicsFixtureDef fixtureDef;
		fixtureDef.shapeType = PhysicsShapeType::Polygon;
		fixtureDef.vertexCount = 4;
		fixtureDef.vertices[0].set(-hx, -hy);
		fixtureDef.vertices[1].set( hx, -hy);
		fixtureDef.vertices[2].set( hx / 2.0f,  hy);
		fixtureDef.vertices[3].set(-hx / 2.0f,  hy);
		fixtureDef.density = DENSITY_PLAYER;
		fixtureDef.friction = 1.0f;
		body.createFixture(fixtureDef);
		body.setGravityScale(gravityScale);
	}

	addBody(body);
	addBody(circleBody);

	PhysicsRevoluteJointDef revoluteJointDef;
	revoluteJointDef.useWorldPivot = true;
	revoluteJointDef.worldPivot = pos;
	revoluteJointDef.bodyA = circleBody;
	revoluteJointDef.bodyB = body;
	revoluteJointDef.lowerAngle = (float)DegreesToRadians(-10);
	revoluteJointDef.upperAngle = (float)DegreesToRadians(10);
	revoluteJointDef.enableLimit = true;
	revoluteJointDef.collideConnected = false;
	revoluteJointDef.enableMotor = true;
	revoluteJointDef.motorSpeed = 10.0f;
	revoluteJointDef.maxMotorTorque = 100.0f;
	_revoluteJoint = world->createRevoluteJoint(revoluteJointDef);
}

bool Player::isCloseOverSolid (float distance) const
{
	PhysicsVec2 end = getPos();
	end.y += distance;
	IEntity* entity = nullptr;
	_map.rayTrace(getPos(), end, &entity);
	if (entity != nullptr && entity->isSolid())
		return true;

	return false;
}

bool Player::isLanded () const
{
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
