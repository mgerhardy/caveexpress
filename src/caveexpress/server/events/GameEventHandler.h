#pragma once

#include "common/NonCopyable.h"
#include "network/INetwork.h"
#include "common/Animation.h"
#include "common/ErrorTypes.h"
#include "common/SoundType.h"
#include "common/MapFailedReason.h"
#include "common/ThemeType.h"
#include <string>

struct b2Vec2;
class ByteStream;
class ServiceProvider;
class EntityType;

namespace caveexpress {

// forward decl
class NPC;
class Player;
class Water;
class Map;
class IEntity;
class NPCFriendly;

class GameEventHandler: public NonCopyable {
private:
	ServiceProvider* _serviceProvider;

	GameEventHandler ();

	inline void updateParticle (int clientMask, const IEntity& entity) const;

public:
	virtual ~GameEventHandler ();

	void init (ServiceProvider& serviceProvider);

	static GameEventHandler& get ();

	// called each time a new map is loaded
	// this allows the client to reset the scene and prepare for a new map
	void closeMap () const;

	void finishedMap (const std::string& mapname, uint32_t finishPoints, uint32_t time, uint8_t stars) const;
	void failedMap (ClientId clientId, const std::string& mapName, const MapFailedReason& reason, const ThemeType& theme) const;

	// announce the add of a tile to the client
	// these are fixed tiles - the position will not change - called on map load
	void addEntity (int clientMask, const IEntity& entity) const;

	// inform the clients that the given entity was removed from the map
	void removeEntity (int clientMask, const IEntity& entity, bool fadeOut = false) const;

	// tell the client that the given entity id is a cave and give it the state
	void addCave (int clientMask, int id, int caveNumber, bool state) const;
	// send the state of a cave (including the windows) to the client
	void sendLightState (int clientMask, int id, bool state) const;

	void updateEntity (int clientMask, const IEntity& entity) const;

	void updateHitpoints (const Player& player) const;

	void updateLives (const Player& player) const;

	// informs the client about the cave the npc wanna go
	void announceTargetCave (int clientMask, const NPCFriendly& npc, int16_t delayMillis) const;

	// inform the client about the target cave number the npc wants to get carried to
	void setTargetCave (int clientMask, uint8_t number) const;

	// inform the clients that the given entity changed its animation
	// used to e.g. change a npc animation to walking
	void changeAnimation (int clientMask, const IEntity& entity, const Animation& animation) const;

	void sendWaterUpdate (int clientMask, const Water& map) const;

	// sends a water impact event to all of the clients
	// @param[in] x The position on the water surface where the impact happened (in box2d scaling)
	// @param[in] force a factor for the water impact. 1.0f is full force.
	void sendWaterImpact (float x, float force) const;

	void sendRumble (float strength, int lengthMillis) const;

	void sendTimeRemaining (uint16_t seconds) const;

	// inform the client that the current loaded map has failed and is going to get restarted
	void restartMap (int delay) const;

	void backToMain (const std::string& window = "") const;

	void sendCollectState (ClientId clientId, const EntityType &entityType, bool collected);

	void notifyPause (bool pause);

	void sendRope (int clientMask, uint16_t entityId1, uint16_t entityId2);
	void removeRope (int clientMask, uint16_t entityId);
};

#define GameEvent GameEventHandler::get()

}
