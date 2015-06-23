#pragma once

#include "INPCCave.h"

namespace caveexpress {

class NPCFriendly: public INPCCave {
private:
	// used to measure the needed time to bring the npc to its target cave
	uint32_t _collectingTime;
	TimerID _fallingTimer;
	uint32_t _waitPatience;
	uint16_t _swimmingDistance;
	uint16_t _playerId;
	bool _returnToCaveOnIdle;
	CaveMapTile *_targetCave;

public:
	NPCFriendly (CaveMapTile *cave, const EntityType& type = EntityType::NONE, bool returnToCaveOnIdle = false);
	virtual ~NPCFriendly ();

	uint32_t getWaitPatience () const;

	void setTargetCave (CaveMapTile *targetCave);
	CaveMapTile* getTargetCave () const;
	uint8_t getTargetCaveNumber() const;

	void setTargetPlayer (uint16_t playerId);
	uint16_t getTargetPlayer () const;

	// returns true if the movement should get triggered. This usually happens after a short
	// delay in which the npc tells the player where he wanna go
	bool triggerTargetCaveAnnouncement (const b2Vec2& playerPos);
	bool setArrived (const b2Vec2& targetPos);

	bool updateCollectedState ();
	void setCollected ();

	// NPC
	void onContact (b2Contact* contact, IEntity* entity) override;
	bool shouldCollide (const IEntity* entity) const override;

	// ICaveNPC
	void update (uint32_t deltaTime) override;
};

inline uint32_t NPCFriendly::getWaitPatience () const
{
	return _waitPatience;
}

inline void NPCFriendly::setTargetCave (CaveMapTile *targetCave)
{
	_targetCave = targetCave;
}

inline CaveMapTile* NPCFriendly::getTargetCave () const
{
	return _targetCave;
}

inline void NPCFriendly::setCollected ()
{
	if (_collectingTime == 0)
		_collectingTime = _time;
	Log::info(LOG_SERVER, String::format("collected npc %i", _id));
}

inline void NPCFriendly::setTargetPlayer (uint16_t playerId)
{
	_playerId = playerId;
}

inline uint16_t NPCFriendly::getTargetPlayer () const
{
	return _playerId;
}

}
