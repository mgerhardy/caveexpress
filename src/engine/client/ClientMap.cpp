#include "engine/client/ClientMap.h"
#include "engine/common/MapSettings.h"
#include "engine/common/network/messages/StopMovementMessage.h"
#include "engine/common/network/messages/MovementMessage.h"
#include "engine/common/network/messages/FingerMovementMessage.h"
#include "engine/common/network/messages/ClientInitMessage.h"
#include "engine/client/entities/ClientMapTile.h"
#include "engine/client/ui/UI.h"
#include "engine/common/IFrontend.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/client/sound/Sound.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/EventHandler.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/GLShared.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/DateUtil.h"
#include "engine/common/Commands.h"
#include "engine/common/Logger.h"
#include <SDL.h>

ClientMap::ClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider, int referenceTileWidth) :
		IMap(), _x(x), _y(y), _width(width), _height(height), _scale(referenceTileWidth), _restartDue(0), _restartInitialized(0), _mapWidth(
				0), _mapHeight(0), _player(nullptr), _time(0), _playerID(0), _frontend(frontend), _pause(false), _serviceProvider(
						serviceProvider), _screenRumble(false), _screenRumbleStrength(0.0f), _screenRumbleOffsetX(
						0), _screenRumbleOffsetY(0), _particleSystem(
				Config.getClientSideParticleMaxAmount()), _tutorial(false), _started(false), _theme(&ThemeTypes::ROCK)
{
}

ClientMap::~ClientMap ()
{
	close();
}

void ClientMap::close ()
{
	resetCurrentMap();
}

void ClientMap::start ()
{
	_started = true;
}

bool ClientMap::isStarted () const
{
	return !_serviceProvider.getNetwork().isMultiplayer() || _started;
}

void ClientMap::resetCurrentMap ()
{
	_timeManager.reset();
	_settings.clear();
	_name.clear();
	_time = 0;
	_started = false;
	_introWindow = "";
	_tutorial = false;
	_mapWidth = 0;
	_mapHeight = 0;
	_entities.clear();
	_player = nullptr;
	_pause = false;
	_playerID = 0;
	_camera.update(vec2_zero, 0);
	_restartDue = 0;
	_restartInitialized = 0;
	_particleSystem.clear();
	_screenRumble = false;
	_screenRumbleStrength = 0.0f;
	_screenRumbleOffsetX = 0;
	_screenRumbleOffsetY = 0;
	SoundControl.haltAll();
}

void ClientMap::disconnect ()
{
	info(LOG_CLIENT, "send disconnect to server");
	const DisconnectMessage msg;
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
	network.closeClient();
	SoundControl.haltAll();
}

ClientEntityPtr ClientMap::getEntity (uint16_t id)
{
	ClientEntityMapIter iter = _entities.find(id);
	if (iter == _entities.end()) {
		return ClientEntityPtr();
	}
	return iter->second;
}

void ClientMap::removeEntity (uint16_t id, bool fadeOut)
{
	if (fadeOut) {
		ClientEntityPtr e = getEntity(id);
		if (e) {
			e->initFadeOut();
		}
	} else {
		const ClientEntityPtr& e = getEntity(id);
		if (e) {
			e->remove();
			_entities.erase(id);
		}
	}
	if (_playerID == id) {
		info(LOG_CLIENT, String::format("remove client side player with the id %i", id));
		_player = nullptr;
		_playerID = 0;
	}
}

void ClientMap::renderFadeOutOverlay (int x, int y) const
{
	const uint32_t now = _time;
	const uint32_t delay = _restartDue - _restartInitialized;
	const float restartFadeStepWidth = 1.0 / delay;
	const uint32_t delta = now > _restartDue ? 0U : _restartDue - now;
	const float alpha = 1.0 - delta * restartFadeStepWidth;
	const Color color = { 0.0, 0.0, 0.0, alpha };
	_frontend->renderFilledRect(x + _x, y + _y, 0, 0, color);
}

void ClientMap::renderLayer (int x, int y, Layer layer) const
{
	const int mapPosX = x + _screenRumbleOffsetX;
	const int mapPosY = y + _screenRumbleOffsetY;

	for (ClientEntityMapConstIter iter = _entities.begin(); iter != _entities.end(); ++iter) {
		const ClientEntityPtr e = iter->second;
		e->render(_frontend, layer, _scale, mapPosX, mapPosY);
	}
}

void ClientMap::getLayerOffset (int &x, int &y) const
{
	const int mapPixelWidth = _mapWidth * _scale;
	const int screenPixelWidth = _frontend->getWidth();
	const int mapPixelHeight = _mapHeight * _scale;
	const int screenPixelHeight = _frontend->getHeight();
	if (mapPixelWidth < screenPixelWidth) {
		x += screenPixelWidth / 2 - mapPixelWidth / 2;
	}

	if (mapPixelHeight < screenPixelHeight) {
		y += screenPixelHeight / 2 - mapPixelHeight / 2;
	}
}

void ClientMap::render (int x, int y) const
{
	ExecutionTime renderTime("ClientMapRender", 2000L);
	const int baseX = x + _x + _camera.getViewportX();
	const int baseY = y + _y + _camera.getViewportY();
	int layerX = baseX;
	int layerY = baseY;
	getLayerOffset(layerX, layerY);

	_frontend->enableScissor(layerX, layerY,
			std::min(getWidth() - layerX, _mapWidth * _scale),
			std::min(getHeight() - layerY, _mapHeight * _scale));
	renderLayer(layerX, layerY, LAYER_BACK);
	renderLayer(layerX, layerY, LAYER_MIDDLE);
	renderLayer(layerX, layerY, LAYER_FRONT);

	if (_restartDue != 0) {
		renderFadeOutOverlay(x, y);
	}

	Config.setDebugRendererData(layerX, layerY, getWidth(), getHeight(), _scale);
	Config.getDebugRenderer().render();

	_particleSystem.render(_frontend, layerX, layerY);

	_frontend->disableScissor();
}

void ClientMap::init (uint16_t playerID)
{
	info(LOG_CLIENT, String::format("init client map for player %i", playerID));

	_camera.init(getWidth(), getHeight(), _mapWidth, _mapHeight, _scale);

	_restartInitialized = 0U;
	_restartDue = 0U;
	_playerID = playerID;

	const std::string& name = Config.getName();
	const ClientInitMessage msgInit(name);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msgInit);
}

TexturePtr ClientMap::loadTexture (const std::string& name) const
{
	return UI::get().loadTexture(name);
}

bool ClientMap::wantInformation (const EntityType& type) const
{
	return isTutorial();
}

void ClientMap::accelerate (Direction dir) const
{
	const MovementMessage msg(dir);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

void ClientMap::resetAcceleration () const
{
	static const StopFingerMovementMessage msg;
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

void ClientMap::setAcceleration (int dx, int dy) const
{
	const FingerMovementMessage msg(dx, dy);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

void ClientMap::resetAcceleration (Direction dir) const
{
	const StopMovementMessage msg(dir);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

bool ClientMap::initWaitingForPlayer () {
	INetwork& network = _serviceProvider.getNetwork();
	if (network.isMultiplayer())
		return false;

	if (!_introWindow.empty()) {
		UI::get().push(_introWindow);
	} else {
		Commands.executeCommandLine(CMD_START);
	}

	return true;
}

void ClientMap::update (uint32_t deltaTime)
{
	if (isPause())
		return;

	if (_screenRumble) {
		_screenRumbleOffsetX = _screenRumbleOffsetY = 0;
		_screenRumbleOffsetX = rand() % std::max(2, static_cast<int>(_screenRumbleStrength * 10.0f));
		_screenRumbleOffsetY = rand() % std::max(2, static_cast<int>(_screenRumbleStrength * 10.0f));
	}

	_timeManager.update(deltaTime);
	_particleSystem.update(deltaTime);

	_time += deltaTime;
	if (_player) {
		_camera.update(_player->getPos(), _player->getMoveDirection());
		SoundControl.setListenerPosition(_player->getPos());
	}
	const ExecutionTime updateTime("ClientMap", 2000L);
	for (ClientEntityMapIter i = _entities.begin(); i != _entities.end();) {
		const bool val = i->second->update(deltaTime, _restartDue == 0);
		if (!val) {
			_entities.erase(i++);
		} else {
			++i;
		}
	}
}

bool ClientMap::load (const std::string& name, const std::string& title)
{
	info(LOG_CLIENT, "load map " + name);
	resetCurrentMap();
	_name = name;
	_title = title;

	return true;
}

void ClientMap::addEntity (ClientEntityPtr e)
{
	_entities[e->getID()] = e;
	if (e->getID() == _playerID) {
		_player = static_cast<ClientPlayer*>(e.get());
	}

	debug(LOG_CLIENT, "add entity " + e->getType().name + " - " + string::toString((int)e->getID()));
}

void ClientMap::setSetting (const std::string& key, const std::string& value)
{
	debug(LOG_CLIENT, "client key: " + key + " = " + value);
	_settings[key] = value;

	if (key == msn::WIDTH) {
		_mapWidth = string::toInt(value);
	} else if (key == msn::HEIGHT) {
		_mapHeight = string::toInt(value);
	} else if (key == msn::THEME) {
		_theme = &ThemeType::getByName(value);
	} else if (key == msn::TUTORIAL) {
		_tutorial = string::toBool(value);
	} else if (key == msn::INTROWINDOW) {
		_introWindow = value;
	}
}

void ClientMap::couldNotFindEntity (const std::string& prefix, uint16_t id) const
{
	info(LOG_CLIENT, String::format("could not find entity with the id %i", (int)id) + " in " + prefix);
}

void ClientMap::changeAnimation (uint16_t id, const Animation& animation)
{
	const ClientEntityMapIter& iter = _entities.find(id);
	if (iter != _entities.end()) {
		iter->second->setAnimationType(animation);
		return;
	}
	couldNotFindEntity("changeAnimation", id);
}

bool ClientMap::updateEntity (uint16_t id, float x, float y, EntityAngle angle, uint8_t state)
{
	const ClientEntityMapIter& iter = _entities.find(id);
	if (iter != _entities.end()) {
		iter->second->setPos(vec2(x, y));
		iter->second->setAngle(angle);
		iter->second->changeState(state);
		return true;
	}
	couldNotFindEntity("updateEntity", id);
	return false;
}

void ClientMap::onData (ByteStream &data)
{
	while (!data.empty()) {
		const ScopedPtr<IProtocolMessage> msg(ProtocolMessageFactory::get().create(data));
		if (!msg) {
			error(LOG_SERVER, "no message for type " + string::toString(static_cast<int>(data.readByte())));
			continue;
		}

		IClientProtocolHandler* handler = ProtocolHandlerRegistry::get().getClientHandler(*msg);
		if (handler != nullptr)
			handler->execute(*msg);
		else
			error(LOG_SERVER, String::format("no client handler for message type %i", msg->getId()));
	}
}

void ClientMap::disableScreenRumble ()
{
	info(LOG_CLIENT, "stop rumble on the screen");
	_screenRumble = false;
	_screenRumbleStrength = 0.0f;
	_screenRumbleOffsetX = _screenRumbleOffsetY = 0;
}

void ClientMap::rumble (float strength, int lengthMillis)
{
	_frontend->rumble(strength, lengthMillis);
	info(LOG_CLIENT, "rumble on the screen: " + string::toString(strength));
	_screenRumble = true;
	_screenRumbleStrength = strength;
	_timeManager.setTimeout(lengthMillis, this, &ClientMap::disableScreenRumble);
}

void ClientMap::spawnInfo (const vec2& position, const EntityType& type)
{
	// TODO:
}
