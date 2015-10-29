#include "NPCFriendly.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/CaveMapTile.h"
#include "caveexpress/server/entities/Fruit.h"
#include "caveexpress/server/events/GameEventHandler.h"
#include "caveexpress/shared/CaveExpressSoundType.h"
#include "common/TimeManager.h"

namespace caveexpress {

NPCFriendly::NPCFriendly (CaveMapTile *cave, const EntityType& type, bool returnToCaveOnIdle) :
		INPCCave(cave, type, false), _collectingTime(0), _fallingTimer(-1), _playerId(-1), _returnToCaveOnIdle(returnToCaveOnIdle), _targetCave(nullptr)
{
	_swimmingTime = 0;
	_swimmingTimeDelay = 7000;
	_initialSwimmingSpeed = 0.8f;
	if (EntityTypes::isNpcGrandpa(_type)) {
		_swimmingTimeDelay = 5000;
		_initialSwimmingSpeed = 0.6f;
		_swimmingDistance = 3;
		_waitPatience = 16000;
	} else if (EntityTypes::isNpcMan(_type)){
		_waitPatience = 8000;
	} else if (EntityTypes::isNpcWoman(_type)){
		_waitPatience = 12000;
	} else {
		_waitPatience = 10000;
		_swimmingDistance = 20;
	}
}

NPCFriendly::~NPCFriendly ()
{
	_map.getTimeManager().clearTimeout(_fallingTimer);
}

bool NPCFriendly::shouldCollide (const IEntity* entity) const
{
	if (isArrived()) {
		if (entity->isCave())
			return true;
		return entity->isSolid();
	}

	return INPCCave::shouldCollide(entity);
}

void NPCFriendly::onContact (b2Contact* contact, IEntity* entity)
{
	INPCCave::onContact(contact, entity);

	// TODO: if you touch a platform because the water is going down again - switch from swimming to idle

	if (entity->isCave()) {
		setDone();
		CaveMapTile *cave = static_cast<CaveMapTile*>(entity);
		cave->setRespawnPossible(true, getType());
	} else if (entity->isPlayer()) {
		if (isMoving() || isSwimming()) {
			Player *player = static_cast<Player*>(entity);
			if (isSwimming())
				_map.sendSound(player->getVisMask(), SoundTypes::SOUND_NPC_CAVE_WATER_RESCUE, player->getPos());
			else
				_map.sendSound(player->getVisMask(), SoundTypes::SOUND_NPC_CAVE_BOARD, player->getPos());
			player->setCollectedNPC(this);
			setState(NPCState::NPC_COLLECTED);
		} else if (isStruggle()) {
			setDying(nullptr);
		} else if (isIdle()) {
			// hit by a player - so we will fall into the water
			setAnimationType(getFallingAnimation());
			_fallingTimer = _map.getTimeManager().setTimeout(500, static_cast<NPC*>(this), &NPC::setFalling);
		}
	}
}

bool NPCFriendly::triggerTargetCaveAnnouncement (const b2Vec2& playerPos)
{
	const float distance = b2Distance(playerPos, getPos());
	if (isSwimming() && distance > _swimmingDistance) {
		return false;
	}
	if (_triggerMovement == 0) {
		_triggerMovement = _time + 400;
		GameEvent.announceTargetCave(getVisMask(), *this, 2000);
		return false;
	}
	return _time > _triggerMovement;
}

bool NPCFriendly::updateCollectedState ()
{
	const Map::PlayerList& players = _map.getPlayers();
	for (Map::PlayerListConstIter i = players.begin(); i != players.end(); ++i) {
		Player* player = *i;
		if (!player->isTransfering(this))
			continue;
		if (!player->isLandedOn(getTargetCave()))
			continue;

		const float start = getTargetCave()->getPlatformStartGridX();
		const float end = getTargetCave()->getPlatformEndGridX();
		b2Vec2 pos = player->getPos();
		pos.x = clamp(pos.x, start, end);
		createBody(pos, _map.getWorld(), true);
		player->setCollectedNPC(nullptr);
		const b2Vec2& targetPos = getTargetCave()->getPos();
		const bool bonus = setArrived(targetPos);
		Log::debug(LOG_GAMEIMPL, "landed on target cave and (re-)spawned npc with id %i", getID());
		if (bonus) {
			_map.addPoints(player, 20);
			Fruit* entity = new Fruit(_map, EntityTypes::APPLE, getPos().x, getPos().y);
			entity->createBody();
		}
		return bonus;
	}

	return false;
}

uint8_t NPCFriendly::getTargetCaveNumber () const
{
	return getTargetCave()->getCaveNumber();
}

bool NPCFriendly::setArrived (const b2Vec2& targetPos)
{
	setMoving(targetPos);
	setState(NPCState::NPC_ARRIVED);
	const uint32_t deltaSeconds = _time - _collectingTime;
	Log::info(LOG_GAMEIMPL, "took %ims to transfer the npc %i to its target cave", deltaSeconds, _id);
	const int bonus = deltaSeconds < 4000;
	_collectingTime = 0;
	return bonus;
}

void NPCFriendly::update (uint32_t deltaTime)
{
	INPCCave::update(deltaTime);

	// return to start (cave) if the npc is idling (the player left the landing spot)
	if (_returnToCaveOnIdle && isIdle()) {
		returnToInitialPosition();
	}

	if (isDone()) {
		_map.removeNPC(this, true);
		_remove = true;
		_map.countTransferedNPC();
		return;
	}

	const CaveMapTile *targetCave = getTargetCave();
	if (targetCave != nullptr && targetCave->isUnderWater()) {
		CaveMapTile *cave = _map.getTargetCave(getCave());
		if (cave == nullptr) {
			_map.restart(2000);
			return;
		}
		setTargetCave(cave);
	}

	if (isCollected()) {
		updateCollectedState();
		return;
	}

	const Map::PlayerList& players = _map.getPlayers();
	for (Map::PlayerListConstIter i = players.begin(); i != players.end(); ++i) {
		Player* player = *i;
		if (player->isFree() && player->isLandedOn(getCave())) {
			// player is landed on our tile - walk toward him
			if (isIdle()) {
				if (triggerTargetCaveAnnouncement(player->getPos())) {
					setMoving(player->getPos());
					setTargetPlayer(player->getID());
				}
			}
		} else if (isMoving() && getTargetPlayer() == player->getID()) {
			// player is not landed, so stop the movement
			setIdle();
			setTargetPlayer(-1);
		} else if (isStruggle() && player->isTouchingWater() && player->isFree()) {
			if (triggerTargetCaveAnnouncement(player->getPos())) {
				setSwimming(player->getPos());
				setTargetPlayer(player->getID());
			}
		} else if (isSwimming() && !player->isTouchingWater()) {
			if (getTargetPlayer() == player->getID()) {
				setSwimmingIdle();
				setTargetPlayer(-1);
			}
		}
	}
}

}
