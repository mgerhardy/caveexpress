#pragma once

#include "engine/client/shaders/WaterShader.h"
#include "engine/client/entities/ClientPlayer.h"
#include "engine/client/particles/IParticleEnvironment.h"
#include "engine/client/Camera.h"
#include "engine/common/Animation.h"
#include "engine/common/IEventObserver.h"
#include "engine/common/IMap.h"
#include "engine/common/ConfigVar.h"
#include "engine/common/ThemeType.h"
#include "engine/common/TimeManager.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/Direction.h"
#include "engine/client/particles/ParticleSystem.h"
#include <map>

namespace {
const uint8_t WATER_ALPHA = 120;
const uint8_t WATERCOLOR[] = { 178, 178, 255, WATER_ALPHA };
}

class ClientMap: public IMap, public IClientCallback, public IEventObserver, public IParticleEnvironment {
public:
	typedef std::map<uint16_t, ClientEntityPtr> ClientEntityMap;
	typedef ClientEntityMap::iterator ClientEntityMapIter;
	typedef ClientEntityMap::const_iterator ClientEntityMapConstIter;
protected:
	// node dimensions
	int _x;
	int _y;
	int _width;
	int _height;
	// the reference tile width to convert the grid sizes into pixels
	int _scale;
	float _zoom;

	// all the maptiles and other entities (e.g. stones) in this map
	ClientEntityMap _entities;
	ClientPlayer *_player;

	uint32_t _restartDue;
	uint32_t _restartInitialized;

	// map grid dimensions
	int _mapWidth;
	int _mapHeight;

	uint32_t _time;

	uint16_t _playerID;

	IFrontend *_frontend;

	bool _pause;

	Camera _camera;

	ServiceProvider& _serviceProvider;

	TimeManager _timeManager;

	bool _screenRumble;
	float _screenRumbleStrength;
	int _screenRumbleOffsetX;
	int _screenRumbleOffsetY;

	ParticleSystem _particleSystem;

	bool _tutorial;
	std::string _introWindow;
	bool _started;
	const ThemeType* _theme;

	ConfigVarPtr _minZoom;
	ConfigVarPtr _maxZoom;

	void renderLayer (int x, int y, Layer layer) const;
	void renderFadeOutOverlay () const;
	virtual void couldNotFindEntity (const std::string& prefix, uint16_t id) const;
	void disableScreenRumble ();

	virtual bool wantLerp () { return _restartDue == 0; }

public:
	ClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider, int referenceTileWidth);
	virtual ~ClientMap ();

	virtual void render () const;
	virtual void renderParticles (int x, int y) const;
	virtual void setSetting (const std::string& key, const std::string& value);

	virtual bool secondFinger () { return false; }

	virtual void resetCurrentMap ();
	void close ();
	void setZoom (const float zoom);
	inline float getZoom () const { return _zoom; }
	void disconnect ();
	virtual void init (uint16_t playerID);
	bool load (const std::string& name, const std::string& title);

	void accelerate (Direction dir) const;
	void resetAcceleration (Direction dir) const;

	void setAcceleration (int dx, int dy) const;
	void resetAcceleration () const;

	bool initWaitingForPlayer ();

	bool wantInformation (const EntityType& type) const;
	bool isTutorial () const;

	uint32_t getTime () const;

	void addEntity (ClientEntityPtr entity);
	ClientEntityPtr getEntity (uint16_t id);
	void removeEntity (uint16_t id, bool fadeOut);
	void changeAnimation (uint16_t id, const Animation& animation);
	bool updateEntity (uint16_t id, float x, float y, EntityAngle angle, uint8_t state);
	ClientPlayer* getPlayer () const;
	const ClientEntityMap& getEntities () const;

	bool isPause () const;
	void setPause (bool pause);

	bool isStarted () const;
	virtual void start ();

	void setPos (int x, int y);
	void setSize (int width, int height);

	int getX () const;
	int getY () const;
	int getWidth () const;
	int getHeight () const;
	const ThemeType& getTheme () const;

	Camera& getCamera ();

	void rumble (float strength, int lengthMillis);
	void spawnInfo (const vec2& position, const EntityType& type);

	int getWaterWidth () const override { return _mapWidth * _scale; }
	int getPixelWidth () const override { return _mapWidth * _scale; }
	int getPixelHeight () const override { return _mapHeight * _scale; }
	int getRenderOffsetX() const override { return _x + _camera.getViewportX(); }
	int getRenderOffsetY() const override { return _y + _camera.getViewportY(); }
	TexturePtr loadTexture (const std::string& name) const override;

	// IMap
	virtual void update (uint32_t deltaTime) override;
	bool isActive () const override;
	void restart (uint32_t delay) override;
	int getMapWidth () const override;
	int getMapHeight () const override;

	// IClientCallback
	void onData (ByteStream &data) override;
};

inline const ThemeType& ClientMap::getTheme () const
{
	return *_theme;
}

inline bool ClientMap::isActive () const
{
	return _mapWidth > 0 && _mapHeight > 0 && !_name.empty();
}

inline Camera& ClientMap::getCamera ()
{
	return _camera;
}

inline void ClientMap::restart (uint32_t delay)
{
	_restartInitialized = _time;
	_restartDue = _time + delay;
}

inline int ClientMap::getMapWidth () const
{
	return _mapWidth;
}

inline int ClientMap::getMapHeight () const
{
	return _mapHeight;
}

inline ClientPlayer* ClientMap::getPlayer () const
{
	return _player;
}

inline uint32_t ClientMap::getTime () const
{
	return _time;
}

inline bool ClientMap::isPause () const
{
	return _pause;
}

inline void ClientMap::setPause (bool pause)
{
	_pause = pause;
}

inline void ClientMap::setPos (int x, int y)
{
	_x = x;
	_y = y;
}

inline void ClientMap::setSize (int width, int height)
{
	_width = width;
	_height = height;

	_camera.init(getWidth(), getHeight(), _mapWidth, _mapHeight, _scale);
}

inline int ClientMap::getX () const
{
	return _x;
}

inline int ClientMap::getY () const
{
	return _y;
}

inline int ClientMap::getWidth () const
{
	if (_width == -1)
		return _frontend->getWidth();
	return _width;
}

inline int ClientMap::getHeight () const
{
	if (_height == -1)
		return _frontend->getHeight();
	return _height;
}

inline const ClientMap::ClientEntityMap& ClientMap::getEntities () const
{
	return _entities;
}

inline bool ClientMap::isTutorial () const
{
	return _tutorial;
}
