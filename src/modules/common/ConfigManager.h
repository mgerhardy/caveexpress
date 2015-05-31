#pragma once

#include "common/NonCopyable.h"
#include "common/DebugRendererData.h"
#include "common/ICommand.h"
#include "common/ConfigVar.h"
#include "common/IConfigPersister.h"
#include <string>
#include <map>

typedef std::map<std::string, ConfigVarPtr> ConfigVarsMap;
typedef ConfigVarsMap::const_iterator ConfigVarsMapConstIter;
typedef ConfigVarsMap::iterator ConfigVarsMapIter;

// forward decl
class LUA;
class IBindingSpaceListener;

typedef void (*backendRenderer) (void *userdata);
class BackendRendererData {
public:
	bool activate;
	backendRenderer renderer;
	void *userdata;

	void render ()
	{
		if (!activate)
			return;
		renderer(userdata);
	}
};

typedef enum {
	BINDINGS_UI,
	BINDINGS_MAP,
	BINDINGS_MAX
} BindingSpace;

typedef enum {
	KEYBOARD,
	JOYSTICK,
	CONTROLLER
} BindingType;

class ConfigManager: public NonCopyable {
private:
	ConfigManager ();

	typedef std::map<int, std::string> KeyBindingMap;
	KeyBindingMap _keybindings[BINDINGS_MAX];

	typedef std::map<int, std::string> JoystickBindingMap;
	JoystickBindingMap _joystickBindings[BINDINGS_MAX];

	typedef std::map<std::string, std::string> ControllerBindingMap;
	ControllerBindingMap _controllerBindings[BINDINGS_MAX];

	typedef std::map<std::string, std::string> KeyValueMap;
	KeyValueMap _configVarMap;

	typedef std::map<int, int> KeyModifierMap;
	KeyModifierMap _keyModifiers;

	ConfigVarsMap _configVars;
	IConfigPersister *_persister;

	ConfigVarPtr _width;
	ConfigVarPtr _height;
	ConfigVarPtr _port;
	ConfigVarPtr _debug;
	ConfigVarPtr _network;
	ConfigVarPtr _grabMouse;
	ConfigVarPtr _joystick;
	ConfigVarPtr _language;
	ConfigVarPtr _fullscreen;
	ConfigVarPtr _soundEnabled;
	ConfigVarPtr _showFPS;
	ConfigVarPtr _soundEngine;
	ConfigVarPtr _shader;
	ConfigVarPtr _name;
	ConfigVarPtr _vsync;
	ConfigVarPtr _textureSize;
	ConfigVarPtr _referenceTimeFactor;
	ConfigVarPtr _maxHitpoints;
	ConfigVarPtr _damageThreshold;
	ConfigVarPtr _fruitCollectDelayForANewLife;
	ConfigVarPtr _amountOfFruitsForANewLife;
	ConfigVarPtr _fruitHitpoints;
	ConfigVarPtr _npcFlyingSpeed;
	ConfigVarPtr _debugui;
	ConfigVarPtr _mode;
	ConfigVarPtr _serverName;
	// server side water particles
	ConfigVarPtr _waterParticle;
	// client side particle amount
	ConfigVarPtr _particles;

	BindingSpace _bindingSpace;

	DebugRendererData _debugRendererData;

	BackendRendererData _backendRenderer;

	IBindingSpaceListener *_bindingSpaceListener;

	void getBindingMap (LUA& lua, std::map<int, std::string>* map, const char *key, BindingType type);
	void getBindingMap (LUA& lua, std::map<std::string, std::string>* map, const char *key, BindingType type);

	void getKeyValueMap (LUA& lua, std::map<std::string, std::string>& map, const char *key);

	int mapModifier (const std::string& name);
	int mapKey (const std::string& name);

	std::string getNameForKey (int key) const;
	std::string getNameForJoystickButton (int key) const;
	std::string getNameForControllerButton (int key) const;

	ConfigVarPtr getConfigValue (KeyValueMap &map, const std::string& name, const std::string& defaultValue = "", unsigned int flags = 0U);
public:

	virtual ~ConfigManager ();

	static ConfigManager& get ();

	void init (IBindingSpaceListener *bindingSpaceListener, int argc, char **argv);
	void shutdown ();

	int getClientSideParticleMaxAmount () const;
	// get the width of the screen in pixels
	int getWidth () const;
	// get the height of the screen in pixels
	int getHeight () const;
	bool isFullscreen () const;
	bool isSoundEnabled () const;
	bool toggleSound ();
	bool isJoystick () const;
	bool toggleJoystick ();
	// the network port the server is listening on
	int getPort () const;
	std::string getServerName () const;
	bool showFPS () const;
	const std::string& getSoundEngine () const;
	const std::string& getLanguage () const;
	void setDebugRendererData (int mapX, int mapY, int width, int height, int scale);
	const DebugRendererData& getMapDebugRect () const;
	const std::string& getName () const;
	bool isShader () const;
	bool isDebugUI () const;
	bool isDebug () const;
	bool isGrabMouse () const;
	bool isVSync () const;
	bool isNetwork () const;
	const std::string& getTextureSize () const;
	void setTextureSize (const std::string& textureSize) const;
	float getReferenceTimeFactor () const;
	int getMaxHitpoints () const;
	float getDamageThreshold () const;
	int getAmountOfFruitsForANewLife () const;
	int getFruitHitpoints () const;
	float getNpcFlyingSpeed () const;
	int getFruitCollectDelayForANewLife () const;
	bool useWaterParticles () const;

	bool isModeSelected () const;
	bool isModeEasy () const;
	bool isModeHard () const;
	void setMode (const std::string& mode);
	void setBindingsSpace (BindingSpace bindingSpace);
	BindingSpace getBindingsSpace () const;

	inline BackendRendererData getDebugRenderer ()
	{
		return _backendRenderer;
	}

	inline void setDebugRenderer (bool activate, backendRenderer renderer = nullptr, void *userdata = nullptr)
	{
		_backendRenderer.activate = activate;
		_backendRenderer.renderer = renderer;
		_backendRenderer.userdata = userdata;
	}

	inline int getKeyModifier (int key) const
	{
		KeyModifierMap::const_iterator iter = _keyModifiers.find(key);
		if (iter == _keyModifiers.end()) {
			return 0;
		}
		return iter->second;
	}

	inline std::string getKeyBinding (int key) const
	{
		KeyBindingMap::const_iterator iter = _keybindings[_bindingSpace].find(key);
		if (iter == _keybindings[_bindingSpace].end()) {
			return "";
		}
		return iter->second;
	}

	inline std::string getJoystickBinding (int key) const
	{
		JoystickBindingMap::const_iterator iter = _joystickBindings[_bindingSpace].find(key);
		if (iter == _joystickBindings[_bindingSpace].end()) {
			return "";
		}
		return iter->second;
	}

	inline std::string getControllerBinding (const std::string& button) const
	{
		ControllerBindingMap::const_iterator iter = _controllerBindings[_bindingSpace].find(button);
		if (iter == _controllerBindings[_bindingSpace].end()) {
			return "";
		}
		return iter->second;
	}

	// ICommand binding
	void setConfig (const ICommand::Args& args);
	void listConfigVariables (const ICommand::Args& args);

	void autoComplete (const std::string& input, std::vector<std::string>& matches);

	ConfigVarPtr getConfigVar (const std::string& name, const std::string& value = "", bool create = true, unsigned int flags = 0U);
};

inline const DebugRendererData& ConfigManager::getMapDebugRect () const
{
	return _debugRendererData;
}

inline const std::string& ConfigManager::getName () const
{
	return _name->getValue();
}

inline bool ConfigManager::isShader () const
{
	return _shader->getBoolValue();
}

inline bool ConfigManager::isDebugUI () const
{
	return _debugui && _debugui->getBoolValue();
}

inline bool ConfigManager::isDebug () const
{
	return _debug && _debug->getBoolValue();
}

inline bool ConfigManager::isGrabMouse () const
{
	return _grabMouse->getBoolValue();
}

inline int ConfigManager::getWidth () const
{
	return _width->getIntValue();
}

inline int ConfigManager::getHeight () const
{
	return _height->getIntValue();
}

inline bool ConfigManager::isFullscreen () const
{
	return _fullscreen->getBoolValue();
}

inline bool ConfigManager::isSoundEnabled () const
{
	return _soundEnabled->getBoolValue();
}

inline bool ConfigManager::toggleSound ()
{
	if (isSoundEnabled())
		_soundEnabled->setValue("false");
	else
		_soundEnabled->setValue("true");

	return _soundEnabled->getBoolValue();
}

inline bool ConfigManager::isJoystick () const
{
	return _joystick->getBoolValue();
}

inline bool ConfigManager::toggleJoystick ()
{
	if (isJoystick())
		_joystick->setValue("false");
	else
		_joystick->setValue("true");

	return _joystick->getBoolValue();
}

inline int ConfigManager::getPort () const
{
	return _port->getIntValue();
}

inline std::string ConfigManager::getServerName () const
{
	return _serverName->getValue();
}

inline void ConfigManager::setDebugRendererData (int x, int y, int width, int height, int scale)
{
	_debugRendererData.x = x;
	_debugRendererData.y = y;
	_debugRendererData.w = width;
	_debugRendererData.h = height;
	_debugRendererData.scale = scale;
}

inline bool ConfigManager::showFPS () const
{
	return _showFPS->getBoolValue();
}

inline bool ConfigManager::isVSync() const
{
	return _vsync->getBoolValue();
}

inline bool ConfigManager::isNetwork() const
{
	return _network->getBoolValue();
}

inline const std::string& ConfigManager::getSoundEngine () const
{
	return _soundEngine->getValue();
}

inline const std::string& ConfigManager::getLanguage () const
{
	return _language->getValue();
}

inline const std::string& ConfigManager::getTextureSize () const
{
	return _textureSize->getValue();
}

inline void ConfigManager::setTextureSize (const std::string& textureSize) const
{
	return _textureSize->setValue(textureSize);
}

inline float ConfigManager::getReferenceTimeFactor () const
{
	return _referenceTimeFactor->getFloatValue();
}

inline int ConfigManager::getMaxHitpoints () const
{
	return _maxHitpoints->getIntValue();
}

inline float ConfigManager::getDamageThreshold () const
{
	return _damageThreshold->getFloatValue();
}

inline int ConfigManager::getFruitCollectDelayForANewLife () const
{
	return _fruitCollectDelayForANewLife->getIntValue();
}

inline int ConfigManager::getAmountOfFruitsForANewLife () const
{
	return _amountOfFruitsForANewLife->getIntValue();
}

inline int ConfigManager::getFruitHitpoints () const
{
	return _fruitHitpoints->getIntValue();
}

inline float ConfigManager::getNpcFlyingSpeed () const
{
	return _npcFlyingSpeed->getFloatValue();
}

inline bool ConfigManager::isModeSelected () const
{
	return !_mode->getValue().empty();
}

inline bool ConfigManager::isModeHard () const
{
	return _mode->getValue() == "hard";
}

inline bool ConfigManager::isModeEasy () const
{
	return _mode->getValue() == "easy";
}

inline void ConfigManager::setMode (const std::string& mode)
{
	_mode->setValue(mode);
	_persister->save(_configVars);
}

inline BindingSpace ConfigManager::getBindingsSpace () const
{
	return _bindingSpace;
}

inline bool ConfigManager::useWaterParticles () const
{
	return _waterParticle->getBoolValue();
}

inline int ConfigManager::getClientSideParticleMaxAmount () const
{
	return _particles->getIntValue();
}

#define Config ConfigManager::get()
