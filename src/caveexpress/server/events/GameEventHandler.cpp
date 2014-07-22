#include "GameEventHandler.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/network/messages/MapRestartMessage.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "caveexpress/shared/network/messages/RemoveRopeMessage.h"
#include "engine/common/network/messages/PauseMessage.h"
#include "engine/common/network/messages/UpdateEntityMessage.h"
#include "engine/common/network/messages/AddEntityMessage.h"
#include "caveexpress/shared/network/messages/WaterHeightMessage.h"
#include "engine/common/network/messages/ChangeAnimationMessage.h"
#include "caveexpress/shared/network/messages/UpdateHitpointsMessage.h"
#include "caveexpress/shared/network/messages/UpdateLivesMessage.h"
#include "caveexpress/shared/network/messages/UpdateCollectedTypeMessage.h"
#include "caveexpress/shared/network/messages/AddRopeMessage.h"
#include "caveexpress/shared/network/messages/TimeRemainingMessage.h"
#include "engine/common/network/messages/RemoveEntityMessage.h"
#include "caveexpress/shared/network/messages/AddCaveMessage.h"
#include "caveexpress/shared/network/messages/WaterImpactMessage.h"
#include "caveexpress/shared/network/messages/LightStateMessage.h"
#include "engine/common/network/messages/RumbleMessage.h"
#include "engine/common/network/messages/BackToMainMessage.h"
#include "engine/common/network/messages/FinishedMapMessage.h"
#include "engine/common/network/messages/FailedMapMessage.h"
#include "engine/common/network/messages/UpdateParticleMessage.h"
#include "engine/common/network/INetwork.h"

#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/server/entities/Player.h"
#include "caveexpress/server/entities/Water.h"

GameEventHandler::GameEventHandler () :
		_serviceProvider(nullptr)
{
}

GameEventHandler::~GameEventHandler ()
{
}

void GameEventHandler::init (ServiceProvider& serviceProvider)
{
	_serviceProvider = &serviceProvider;
}

GameEventHandler& GameEventHandler::get ()
{
	static GameEventHandler _instance;
	return _instance;
}

void GameEventHandler::updateParticle (int clientMask, const IEntity& entity) const
{
	const WorldParticle& particle = static_cast<const WorldParticle&>(entity);
	std::vector<UpdateParticleEntity> bodies;
	const uint8_t bodyCnt = particle.getBodies().size();
	for (uint8_t i = 0; i < bodyCnt; i++) {
		if (!particle.isActive(i))
			continue;
		const b2Body* body = particle.getBodies()[i];
		const b2Vec2& pos = body->GetPosition();
		const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(body->GetAngle()));
		const UpdateParticleEntity e = { pos.x, pos.y, angle, i, particle.getRemainingLifetime(i) };
		bodies.push_back(e);
	}
	const UpdateParticleMessage msg(entity.getID(), bodies, particle.getMaxParticles(), particle.getLifeTime());
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::addEntity (int clientMask, const IEntity& entity) const
{
	if (entity.isParticle()) {
		updateParticle(clientMask, entity);
		return;
	}
	const b2Vec2& pos = entity.getPos();
	const b2Vec2& size = entity.getSize();
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const AddEntityMessage msg(entity.getID(), entity.getType(), entity.getAnimationType(),
			entity.getSpriteID(), pos.x, pos.y, size.x, size.y, angle, entity.getSpriteAlignment());
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::updateEntity (int clientMask, const IEntity& entity) const
{
	if (entity.isParticle()) {
		updateParticle(clientMask, entity);
		return;
	}
	const b2Vec2& vec = entity.getPos();
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const UpdateEntityMessage msg(entity.getID(), vec.x, vec.y, angle, entity.getState());
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::updateHitpoints (const Player& player) const
{
	const UpdateHitpointsMessage msg(player.getHitpoints());
	_serviceProvider->getNetwork().sendToClient(player.getClientId(), msg);
}

void GameEventHandler::updateLives (const Player& player) const
{
	const UpdateLivesMessage msg(player.getLives());
	_serviceProvider->getNetwork().sendToClient(player.getClientId(), msg);
}

void GameEventHandler::notifyPause (bool pause)
{
	const PauseMessage msg(pause);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void GameEventHandler::sendRope (int clientMask, uint16_t entityId1, uint16_t entityId2)
{
	const AddRopeMessage msg(entityId1, entityId2);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::removeRope (int clientMask, uint16_t entityId)
{
	const RemoveRopeMessage msg(entityId);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::changeAnimation (int clientMask, const IEntity& entity, const Animation& animation) const
{
	const ChangeAnimationMessage msg(entity.getID(), animation);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::removeEntity (int clientMask, const IEntity& entity, bool fadeOut) const
{
	const RemoveEntityMessage msg(entity.getID(), fadeOut);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::restartMap (int delay) const
{
	const MapRestartMessage msg(delay);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void GameEventHandler::backToMain (const std::string& window) const
{
	const BackToMainMessage reset(window);
	_serviceProvider->getNetwork().sendToAllClients(reset);
}

void GameEventHandler::sendCollectState (ClientId clientId, const EntityType &entityType, bool collected)
{
	const UpdateCollectedTypeMessage msg(entityType, collected);
	_serviceProvider->getNetwork().sendToClients(ClientIdToClientMask(clientId), msg);
}

void GameEventHandler::finishedMap (const std::string& mapName, uint32_t finishPoints, uint32_t time, uint8_t stars) const
{
	const FinishedMapMessage msg(mapName, finishPoints, time, stars);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void GameEventHandler::failedMap (ClientId clientId, const std::string& mapName, const MapFailedReason& reason, const ThemeType& theme) const
{
	const FailedMapMessage msg(mapName, reason, theme);
	_serviceProvider->getNetwork().sendToClient(clientId, msg);
}

void GameEventHandler::closeMap () const
{
	const CloseMapMessage msg;
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void GameEventHandler::sendLightState (int clientMask, int id, bool state) const
{
	const LightStateMessage msg(id, state);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::addCave (int clientMask, int id, bool state) const
{
	const AddCaveMessage msg(id, state);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::sendWaterUpdate (int clientMask, const Water& water) const
{
	const WaterHeightMessage msg(water.getHeight());
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void GameEventHandler::sendWaterImpact (float x, float force) const
{
	const WaterImpactMessage msg(x, force);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void GameEventHandler::sendRumble (float strength, int lengthMillis) const
{
	const RumbleMessage msg(strength, lengthMillis);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void GameEventHandler::sendTimeRemaining (uint16_t seconds) const
{
	const TimeRemainingMessage msg(seconds);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}
