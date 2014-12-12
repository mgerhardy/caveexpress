#include "NPC.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/Border.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/shared/CaveExpressAchievement.h"
#include "engine/common/Logger.h"
#include "engine/common/TimeManager.h"
#include "engine/common/SpriteDefinition.h"
#include "caveexpress/shared/constants/Density.h"

NPC::NPC (const EntityType &type, Map& map) :
		IEntity(type, map), _initialWalkingSpeed(1.1f), _targetPos(b2Vec2_zero), _initialPosition(b2Vec2_zero), _lastDirectionRight(
				true), _dazedTime(0), _dazedTimeout(8000), _idleTimer(0), _moveTimer(0)
{
	setState(NPCState::NPC_IDLE);
}

NPC::~NPC ()
{
	TimeManager& t = _map.getTimeManager();
	t.clearTimeout(_moveTimer);
	t.clearTimeout(_idleTimer);
}

void NPC::setPos (const b2Vec2& pos)
{
	IEntity::setPos(pos);
	_initialPosition = pos;
}

void NPC::onSpawn ()
{
	if (getAnimationType() == Animation::NONE)
		setAnimationType(getIdleAnimation());
}

void NPC::onContact (b2Contact* contact, IEntity* entity)
{
	IEntity::onContact(contact, entity);

	if (!isDying())
		return;

	if (!entity->isBorder())
		return;

	Border *b = static_cast<Border*>(entity);
	const float y = getPos().y;
	if (b->isBottom() || y > b->getPos().y) {
		_remove = true;
	}
}

b2BodyType NPC::getBodyType () const
{
	return b2_dynamicBody;
}

b2Body* NPC::createBody (const b2Vec2 &pos, bool setOnGround, bool fixedRotation)
{
	if (!_bodies.empty()) {
		error(LOG_SERVER, "there are already bodies defined for this npc");
		return nullptr;
	}

	b2BodyDef bodyDef;
	bodyDef.type = getBodyType();
	bodyDef.position.Set(pos.x, pos.y);
	b2Body* body = _map.getWorld()->CreateBody(&bodyDef);
	b2PolygonShape dynamicBox;
	const b2Vec2& size = getSize();
	dynamicBox.SetAsBox(size.x / 2.0f, size.y / 2.0f);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = getDensity();
	fixtureDef.friction = 0.0f;
	fixtureDef.restitution = 0.0f;
	body->SetUserData(static_cast<void*>(this));
	body->CreateFixture(&fixtureDef);
	body->SetFixedRotation(fixedRotation);
	addBody(body);

	if (setOnGround) {
		static const b2Vec2 unitdir(0, 1);
		const b2Vec2 end = pos + 1 * unitdir;
		SetOnGroundRayCastCallback c(this);
		_map.getWorld()->RayCast(&c, pos, end);
		setPos(pos);
	}

	_initialPosition = pos;
	return body;
}

void NPC::update (uint32_t deltaTime)
{
	IEntity::update(deltaTime);

	if (isDying())
		return;

	if (_dazedTime > 0 && _time - _dazedTime > _dazedTimeout) {
		if (_lastDirectionRight)
			setAnimationType(Animations::ANIMATION_WAKEUP_RIGHT);
		else
			setAnimationType(Animations::ANIMATION_WAKEUP_LEFT);

		const int length = SpriteDefinition::get().getAnimationLength(_type, getAnimationType());
		if (length > 0) {
			TimeManager& t = _map.getTimeManager();
			_idleTimer = t.setTimeout(length, this, &NPC::setIdle);
		}
		_dazedTime = 0;
	}

	if (isMoving()) {
		const float xPos = getPos().x;
		static const float gap = 0.1f;
		if (Between(xPos, _targetPos.x - gap, _targetPos.x + gap)) {
			// target reached
			setIdle();
		}
	} else if (isDazed() || isIdle()) {
		setLinearVelocity(b2Vec2_zero);
	}
}

bool NPC::shouldCollide (const IEntity* entity) const
{
	if (isDying()) {
		return false;
	}

	if (isDazed()) {
		return entity->isSolid() || entity->isWater();
	}

	if (entity->isWater()) {
		return true;
	}

	if (isFalling()) {
		return entity->isWater();
	}

	if (entity->isPlayer()) {
		const Player* player = static_cast<const Player*>(entity);
		return !player->isCrashed();
	}

	return entity->isSolid();
}

int NPC::handleTurnAnimation (const b2Vec2& targetPos, const Animation& left, const Animation& right)
{
	int length = -1;
	if (targetPos.x > getPos().x) {
		if (_lastDirectionRight)
			length = changeAnimations(right);
		else
			length = changeAnimations(Animations::ANIMATION_TURN_LEFT, right);
		_lastDirectionRight = true;
	} else {
		if (_lastDirectionRight)
			length = changeAnimations(Animations::ANIMATION_TURN_RIGHT, left);
		else
			length = changeAnimations(left);
		_lastDirectionRight = false;
	}
	setLinearVelocity(b2Vec2_zero);
	setState(NPCState::NPC_IDLE);
	return length;
}

void NPC::setFalling ()
{
	debug(LOG_SERVER, String::format("falling npc %i: %s", getID(), _type.name.c_str()));
	setState(NPCState::NPC_FALLING);
	setAnimationType(getFallingAnimation());
}

void NPC::setMoving (gridCoord x)
{
	setMoving(b2Vec2(x, getPos().y));
}

void NPC::move ()
{
	_moveTimer = 0;
	const float yMovement = -0.01f;
	setState(NPCState::NPC_MOVING);
	const float xMovement = _lastDirectionRight ? _initialWalkingSpeed : -_initialWalkingSpeed;
	const b2Vec2 speed(xMovement, yMovement);
	setLinearVelocity(speed);
}

void NPC::setMoving (const b2Vec2& targetPos)
{
	debug(LOG_SERVER, String::format("moving npc %i: %s", getID(), _type.name.c_str()));
	_targetPos = targetPos;
	if (EntityTypes::hasDirection(_type)) {
		const int length = handleTurnAnimation(targetPos, Animations::ANIMATION_WALK_LEFT, Animations::ANIMATION_WALK_RIGHT);
		if (length > 0) {
			TimeManager& t = _map.getTimeManager();
			_moveTimer = t.setTimeout(length, this, &NPC::move);
		} else {
			move();
		}
	} else {
		if (targetPos.x > getPos().x) {
			_lastDirectionRight = true;
			setAnimationType(Animations::ANIMATION_WALK_RIGHT);
		} else {
			_lastDirectionRight = false;
			setAnimationType(Animations::ANIMATION_WALK_LEFT);
		}
		move();
	}
}

void NPC::setIdle ()
{
	debug(LOG_SERVER, String::format("idle npc %i: %s", getID(), _type.name.c_str()));
	setState(NPCState::NPC_IDLE);
	setAnimationType(getIdleAnimation());
	setLinearVelocity(b2Vec2_zero);
	_idleTimer = 0;
}

void NPC::setDone ()
{
	debug(LOG_SERVER, String::format("done npc %i: %s", getID(), _type.name.c_str()));
	setState(NPCState::NPC_DONE);
}

const Animation& NPC::getIdleAnimation () const
{
	return Animations::ANIMATION_IDLE;
}

const Animation& NPC::getFallingAnimation () const
{
	return Animations::ANIMATION_FALLING;
}

void NPC::setDying (const IEntity* entity)
{
	info(LOG_SERVER, String::format("dying npc %i: %s", getID(), _type.name.c_str()));
	setState(NPCState::NPC_DYING);
	if (EntityTypes::isNpcFlying(_type)) {
		Achievements::DAZE_PTERODACTYLS.unlock();
	}
	setAnimationType(getFallingAnimation());
	_map.addPoints(entity, 15);
}

void NPC::setState (int state)
{
	// don't allow any state change if we are dying
	if (_state == NPCState::NPC_DYING) {
		error(LOG_SERVER, String::format("unallowed npc state change to %i for %i (%s)", state, _id, _type.name.c_str()));
		return;
	}

	IEntity::setState(state);
}

void NPC::setDazed (const IEntity* entity)
{
	if (isDazed()) {
		return;
	}

	info(LOG_SERVER, String::format("dazed npc %i: %s", getID(), _type.name.c_str()));
	setState(NPCState::NPC_DAZED);
	setLinearVelocity(b2Vec2_zero);
	if (EntityTypes::isNpcMammut(_type)) {
		Achievements::DAZE_A_MASTODON.unlock();
	} else if (EntityTypes::isNpcFish(_type)) {
		Achievements::DAZE_A_FISH.unlock();
	}
	_map.addPoints(entity, 10);

	if (_lastDirectionRight) {
		changeAnimations(Animations::ANIMATION_KNOCKOUT_RIGHT, Animations::ANIMATION_DAZED_RIGHT);
	} else {
		changeAnimations(Animations::ANIMATION_KNOCKOUT_LEFT, Animations::ANIMATION_DAZED_LEFT);
	}

	_dazedTime = _time;
}

float NPC::getDensity () const
{
	return DENSITY_NPC;
}

bool NPC::returnToInitialPosition ()
{
	static const float gap = 0.1f;
	const float xPos = getPos().x;
	if (!Between(xPos, _initialPosition.x - gap, _initialPosition.x + gap)) {
		setMoving(_initialPosition);
		return false;
	}
	return true;
}
