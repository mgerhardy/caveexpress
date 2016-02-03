#pragma once

#include "client/entities/ClientPlayer.h"
#include "particles/IParticleEnvironment.h"
#include "client/Camera.h"
#include "common/Animation.h"
#include "common/IEventObserver.h"
#include "common/IMap.h"
#include "common/ConfigVar.h"
#include "common/ThemeType.h"
#include "common/Cooldown.h"
#include "common/TimeManager.h"
#include "network/INetwork.h"
#include "common/Direction.h"
#include "ui/BitmapFont.h"
#include "particles/ParticleSystem.h"
#include <vector>
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
	struct CooldownData {
		uint32_t start;
		uint32_t duration;
	};
	typedef std::vector<CooldownData> CooldownVector;
	CooldownVector _cooldowns;
	BitmapFontPtr _font;
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

	// true if this is a tutorial map - might print extra information on player actions
	bool _tutorial;
	// the window id that should get pushed to the stack whenever the map is started
	std::string _introWindow;
	bool _started;
	const ThemeType* _theme;

	ConfigVarPtr _minZoom;
	ConfigVarPtr _maxZoom;

	// how many different start positions are available in this particular map
	int _startPositions;

	virtual void renderLayer (int x, int y, Layer layer) const;
	void renderFadeOutOverlay () const;
	virtual void couldNotFindEntity (const std::string& prefix, uint16_t id) const;
	void disableScreenRumble ();

	virtual bool updateCameraPosition ();

	virtual bool wantLerp () { return _restartDue == 0; }

public:
	ClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider, int referenceTileWidth);
	virtual ~ClientMap ();

	// indicates that a particular cooldown was triggered
	virtual void cooldown (const Cooldown& cooldown);

	virtual void render () const;
	virtual void renderBegin (int x, int y) const;
	virtual void renderEnd (int x, int y) const;
	virtual void renderLayers (int x, int y) const;
	virtual void renderParticles (int x, int y) const;
	/**
	 * @brief allows to let the game put a cooldown description next to the icon that is rendered
	 * @return the pixels that were needed to place the description
	 * @param[in] x, y, w, h are the upper left corner of the cooldown icon
	 */
	virtual int renderCooldownDescription (int cooldownIndex, int x, int y, int w, int h) const;
	virtual void renderCooldowns (int x, int y) const;
	virtual void setSetting (const std::string& key, const std::string& value);

	/**
	 * @return @c true if the second finger triggers any action.
	 */
	virtual bool secondFinger () { return false; }

	virtual void resetCurrentMap ();
	void close ();
	/**
	 * @brief Scrolls the camera by the given pixels
	 */
	virtual void scroll (int relX, int relY);
	/**
	 * @brief Sets the new zoom level to the given level. This value is clamped between the min and max zoom level
	 * (which in turn is defined by a config variable
	 */
	virtual void setZoom (const float zoom);
	inline float getZoom () const { return _zoom; }
	void disconnect ();
	virtual void init (uint16_t playerID);
	/**
	 * @brief Load the given map. This is not loaded from the disk - but comes from the network. So the client
	 * doesn't need to have the map installed just to play it.
	 */
	bool load (const std::string& name, const std::string& title);

	/**
	 * @brief Sets the amount of start positions for this map.
	 */
	void setStartPositions(int startPositions);

	/**
	 * @brief Accelerate with controller or keyboard - the id specifies which controller did the movement.
	 * This can be used at some point to implement split screen rendering and allow one player
	 * per controller
	 * @note In case of keyboard movement, the id is always 0 right now
	 */
	void accelerate (Direction dir, uint8_t id) const;
	/**
	 * @brief See @c accelerate above
	 */
	void resetAcceleration (Direction dir, uint8_t id) const;

	// finger based movement
	void setFingerAcceleration (int dx, int dy) const;
	void stopFingerAcceleration () const;

	/**
	 * @brief Initializes the waiting dialogs for a multiplayer game
	 */
	bool initWaitingForPlayer ();

	/**
	 * @brief The map can decide to show some kind of extra information for a given @c EntityType
	 * @note This is right now in contexts like spawning and collecting of an entity instance.
	 */
	bool wantInformation (const EntityType& type) const;

	/**
	 * @return @c true if this is a tutorial map that should show additional help texts on
	 * spawning or collecting entities
	 */
	bool isTutorial () const;

	/**
	 * @return the map time in milliseconds, this is not running when the map is in pause mode
	 */
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
	virtual void spawnInfo (const vec2& position, const EntityType& type);

	int getWaterWidth () const override { return _mapWidth * _scale; }
	int getPixelWidth () const override { return _mapWidth * _scale; }
	int getPixelHeight () const override { return _mapHeight * _scale; }
	TexturePtr loadTexture (const std::string& name) const override;

	// converts the given x and y screen coordinates into map coordinates that take the scale, shift and so on into account.
	void getMapPixelForScreenPixel (int x, int y, int *outX, int *outY);
	void getMapGridForScreenPixel (int x, int y, int *outX, int *outY);

	virtual void onWindowResize () override {}

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

inline void ClientMap::setStartPositions (int startPositions)
{
	_startPositions = startPositions;
}
