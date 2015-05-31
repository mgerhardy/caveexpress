#include "NPCAttacking.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/map/Map.h"
#include "common/TimeManager.h"

NPCAttacking::NPCAttacking (const EntityType& entityType, Map& map, bool right) :
		NPCAggressive(entityType, map), _attackTimer(0), _returnTimer(0), _attackTarget(nullptr), _platformStartPos(
				-1), _platformEndPos(-1)
{
	_initialWalkingSpeed = 1.4f;
	_lastDirectionRight = right;
}

NPCAttacking::~NPCAttacking ()
{
	TimeManager& t = _map.getTimeManager();
	t.clearTimeout(_attackTimer);
	t.clearTimeout(_returnTimer);
}

inline void NPCAttacking::changeAttackingAnimation (const b2Vec2 &targetPos)
{
	const b2Vec2 speed(_initialWalkingSpeed * 2, -0.01f);
	if (targetPos.x > getPos().x) {
		_lastDirectionRight = true;
		setLinearVelocity(speed);
		setAnimationType(Animations::ANIMATION_ATTACK_RIGHT);
	} else {
		_lastDirectionRight = false;
		setLinearVelocity(-speed);
		setAnimationType(Animations::ANIMATION_ATTACK_LEFT);
	}
}

void NPCAttacking::onPreSolve (b2Contact* contact, IEntity* entity, const b2Manifold* oldManifold)
{
	NPCAggressive::onPreSolve(contact, entity, oldManifold);
	if (!entity->isPlayer())
		return;

	if (!isAttacking() || _returnTimer != 0) {
		contact->SetEnabled(false);
		return;
	}

	// we hit a player - so the player is crashing
	Player* player = static_cast<Player*>(entity);
	if (!player->isCrashed()) {
		if (EntityTypes::isNpcMammut(_type))
			player->setCrashed(CRASH_NPC_MAMMUT);
		else if (EntityTypes::isNpcWalking(_type))
			player->setCrashed(CRASH_NPC_WALKING);
		else
			player->setCrashed(CRASH_DAMAGE);
		_returnTimer = startTimer(1500, &NPCAttacking::walkBack, true);
	}
}

void NPCAttacking::setAttacking ()
{
	_attackTimer = 0;
	setState(NPCState::NPC_ATTACKING);
	changeAttackingAnimation(_targetPos);
}

void NPCAttacking::calculatePlatformDimensions ()
{
	_map.getPlatformDimensions(static_cast<int>(getPos().x), static_cast<int>(getPos().y), &_platformStartPos, &_platformEndPos);
}

void NPCAttacking::checkAttack (Player *player)
{
	// there is already an attack running
	if (_attackTimer != 0)
		return;

	// we are dazed - so we can't attack anything
	if (isDazed()) {
		stopAttack(_attackTarget);
		return;
	}

	// a crashed player does not need another attack
	if (player->isCrashed()) {
		stopAttack(player);
		return;
	}

	if (EntityTypes::isNpcMammut(_type)) {
		// the player has to land for the mammut to attack
		if (!player->isLanded()) {
			stopAttack(player);
			return;
		}
	} else if (!player->isCloseOverSolid()) {
		stopAttack(player);
		// for the other attacking npc the player must only be hovering over the ground
		return;
	}

	const float playerY = player->getPos().y;
	const float npcY = getPos().y;
	const float gap = 0.4f;
	if (!Between(npcY, playerY - gap, playerY + gap)) {
		stopAttack(player);
		return;
	}

	if (!_map.isReachableByWalking(this, player, _platformStartPos, _platformEndPos)) {
		stopAttack(player);
		return;
	}

	attack(player);
}

void NPCAttacking::stopAttack (const Player *player)
{
	if (player == nullptr)
		return;
	if (_attackTarget != player)
		return;
	info(LOG_SERVER, "stop attacking the player");
	_attackTarget = nullptr;
	_returnTimer = startTimer(1500, &NPCAttacking::walkBack, true);
}

void NPCAttacking::attack (Player* player)
{
	// we are already attacking this player
	if (_attackTarget == player)
		return;

	info(LOG_SERVER, "set attacking for " + _type.name);
	const int length = handleTurnAnimation(player->getPos(), Animations::ANIMATION_ATTACK_INIT_LEFT, Animations::ANIMATION_ATTACK_INIT_RIGHT);
	const int initLength = SpriteDefinition::get().getAnimationLength(_type, Animations::ANIMATION_ATTACK_INIT_LEFT);
	const int attackingDelay = length + initLength;
	_targetPos = player->getPos();
	_attackTarget = player;
	_attackTimer = startTimer(attackingDelay, &NPCAttacking::setAttacking, false);
}

TimerID NPCAttacking::startTimer (int length, void(NPCAttacking::*callback) (), bool abortAnimationChanges)
{
	TimeManager& t = _map.getTimeManager();
	if (t.clearTimeout(_attackTimer))
		o("abort attack timer");
	if (t.clearTimeout(_returnTimer))
		o("abort return timer");
	//if (abortAnimationChanges)
	//	resetAnimationChange();
	_attackTimer = _returnTimer = _moveTimer = 0;
	return t.setTimeout(length, this, callback);
}

bool NPCAttacking::returnToInitialPosition ()
{
	const bool ret = NPC::returnToInitialPosition();
	_returnTimer = 0;
	return ret;
}

void NPCAttacking::walkBack ()
{
	returnToInitialPosition();
}

void NPCAttacking::update (uint32_t deltaTime)
{
	NPCAggressive::update(deltaTime);

	if (isDazed()) {
		TimeManager& t = _map.getTimeManager();
		t.clearTimeout(_attackTimer);
		t.clearTimeout(_returnTimer);
		t.clearTimeout(_moveTimer);
		_attackTimer = _returnTimer = _moveTimer = 0;
	}

	// gradual acceleration reduction if we are about to return to the initial position
	if (_returnTimer != 0) {
		const b2Vec2 vel = getLinearVelocity();
		const float desiredVel = vel.x * 0.98f;
		const float velChange = desiredVel - vel.x;
		const float impulse = getMass() * velChange;
		applyLinearImpulse(b2Vec2(impulse, 0.0f));
	}

	// attack player if he is landed
	const Map::PlayerList& players = _map.getPlayers();
	for (Map::PlayerListConstIter i = players.begin(); i != players.end(); ++i) {
		Player* player = *i;
		checkAttack(player);
	}
}
