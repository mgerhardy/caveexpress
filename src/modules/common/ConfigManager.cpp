#include "ConfigManager.h"
#include "common/CommandSystem.h"
#include "common/ConfigVar.h"
#include "common/ConfigPersisterSQL.h"
#include "common/ConfigPersisterNOP.h"
#include "common/IBindingSpaceListener.h"
#include "common/Log.h"
#include "common/LUA.h"
#include "common/System.h"
#include <SDL.h>
#include <SDL_platform.h>
#include <map>

namespace {
#define CMD_LISTVARS "listvars"
#define CMD_SETVAR "set"
const char *KEY_CONFIG_KEYBINDINGS = "keybindings";
const char *KEY_CONFIG_CONTROLLERBINDINGS = "controllerbindings";
}

ConfigManager::ConfigManager() :
		_persister(nullptr), _bindingSpace(BINDINGS_UI), _bindingSpaceListener(nullptr) {
	memset(&_backendRenderer, 0, sizeof(_backendRenderer));
	_logLevel = LogLevel::LEVEL_INFO;
}

void ConfigManager::setBindingsSpace (BindingSpace bindingSpace)
{
	if (_bindingSpace == bindingSpace)
		return;

	if (_bindingSpaceListener)
		_bindingSpaceListener->resetKeyStates();
	// after the keys are reset, we can change the binding space
	_bindingSpace = bindingSpace;
}

void ConfigManager::init (IBindingSpaceListener *bindingSpaceListener, int argc, char **argv)
{
	_persister = new ConfigPersisterSQL();
	Log::info(LOG_COMMON, "init configmanager");

	_persister->init();

	_bindingSpaceListener = bindingSpaceListener;

	LUA lua;

	Log::info(LOG_COMMON, "load config lua file");
	const bool success = lua.load("config.lua");
	if (!success) {
		Log::error(LOG_COMMON, "could not load config");
	}

	if (success) {
		Log::info(LOG_COMMON, "load config values");
		getKeyValueMap(lua, _configVarMap, "settings");
		Log::info(LOG_COMMON, "load keybindings");
		getBindingMap(lua, _keybindings, KEY_CONFIG_KEYBINDINGS, KEYBOARD);
		Log::info(LOG_COMMON, "load controller bindings");
		getBindingMap(lua, _controllerBindings, KEY_CONFIG_CONTROLLERBINDINGS, CONTROLLER);
	}
	_language = getConfigValue(_configVarMap, "language", System.getLanguage());
	_gameController = getConfigValue(_configVarMap, "gamecontroller");
	_gameControllerTriggerAxis = getConfigValue(_configVarMap, "gamecontrollertriggeraxis");
	_showFPS = getConfigValue(_configVarMap, "showfps");
	_width = getConfigValue(_configVarMap, "width", "-1");
	_height = getConfigValue(_configVarMap, "height", "-1");
	_fullscreen = getConfigValue(_configVarMap, "fullscreen", "true");
	_soundEnabled = getConfigValue(_configVarMap, "sound");
	_port = getConfigValue(_configVarMap, "port", "4567");
	_debug = getConfigValue(_configVarMap, "debug", "false");
#ifdef NONETWORK
	_network = getConfigValue(_configVarMap, "network", "false");
#else
	_network = getConfigValue(_configVarMap, "network", "true");
#endif
	_vsync = getConfigValue(_configVarMap, "vsync", "true");
	_textureSize = getConfigValue(_configVarMap, "texturesize", "auto");
	_grabMouse = getConfigValue(_configVarMap, "grabmouse", "true");
	_soundEngine = getConfigValue(_configVarMap, "soundengine", "sdl");
	_particles = getConfigValue(_configVarMap, "particles", "0");
	_renderToTexture = getConfigValue(_configVarMap, "rendertotexture", "1");
	getConfigValue(_configVarMap, "red", "8");
	getConfigValue(_configVarMap, "green", "8");
	getConfigValue(_configVarMap, "blue", "8");
	_serverName = getConfigVar("servername", System.getCurrentUser());
	_name = getConfigVar("name", System.getCurrentUser());
	_debugui = getConfigVar("debugui", "false");
	_debugEntity = getConfigVar("debugentity", "false", true);
	getConfigVar("alreadyrated", "false", true);
	_mode = getConfigValue(_configVarMap, "mode", "");

	for (KeyValueMap::iterator i = _configVarMap.begin(); i != _configVarMap.end(); ++i) {
		getConfigVar(i->first, i->second, true);
	}

	std::vector<std::string> vars;
	_persister->getVars(vars);
	for (std::vector<std::string>::const_iterator i = vars.begin(); i != vars.end(); ++i) {
		getConfigVar(*i);
	}
	for (auto entry : _configVars) {
		Log::info(LOG_COMMON, "'%s' with value '%s'", entry.first.c_str(), entry.second->getValue().c_str());
	}

	memset(&_debugRendererData, 0, sizeof(_debugRendererData));

	Log::info(LOG_COMMON, "mouse grab enabled: %s", _grabMouse->getValue().c_str());
	Log::info(LOG_COMMON, "controller enabled: %s", _gameController->getValue().c_str());
	Log::info(LOG_COMMON, "     sound enabled: %s", _soundEnabled->getValue().c_str());
	Log::info(LOG_COMMON, "     debug enabled: %s", _debug->getValue().c_str());

	Commands.registerCommand("loglevel", bindFunction(ConfigManager::setLogLevel));
	CommandPtr cmd = Commands.registerCommand(CMD_SETVAR, bindFunction(ConfigManager::setConfig));
	cmd->setCompleter([&] (const std::string& input, std::vector<std::string>& matches) {
		for (auto entry : _configVars) {
			if (!string::startsWith(entry.first, input))
				continue;
			matches.push_back(entry.first);
		}
	});

	Commands.registerCommand(CMD_LISTVARS, bindFunction(ConfigManager::listConfigVariables));

	for (int i = 0; i < argc; i++) {
		if (argv[i][0] != '-')
			continue;
		const std::string command = &argv[i][1];
		if (command == CMD_SETVAR) {
			if (i + 2 >= argc)
				continue;
			const std::string var = argv[i + 1];
			const std::string val = argv[i + 2];
			ConfigVarPtr p = getConfigVar(var);
			p->setValue(val);
			*argv[i] = *argv[i + 1] = *argv[i + 2] = '\0';
			i += 2;
		} else if (command == "loglevel") {
			if (i + 1 >= argc)
				continue;
			ICommand::Args a;
			a.push_back(argv[i + 1]);
			setLogLevel(a);
			*argv[i] = *argv[i + 1] = '\0';
			i += 1;
		}
	}
	increaseCounter("launchcount");
	_persister->save(_configVars);
}

ConfigManager::~ConfigManager ()
{
	delete _persister;
}

ConfigManager& ConfigManager::get ()
{
	static ConfigManager _mgr;
	return _mgr;
}

void ConfigManager::getKeyValueMap (LUA& lua, std::map<std::string, std::string>& map, const char *key)
{
	lua.getKeyValueMap(map, key);
}

void ConfigManager::getBindingMap (LUA& lua, std::map<std::string, std::string>* map, const char *key, BindingType type)
{
	if (!lua.getGlobalKeyValue(key))
		return;

	while (lua.getNextKeyValue()) {
		const std::string id = lua.getKey();
		if (id.empty()) {
			lua.pop();
			continue;
		}

		lua_pushnil(lua.getState());

		std::map<std::string, std::string> strMap;
		while (lua.getNextKeyValue()) {
			const std::string& _key = lua.getLuaValue(-2);
			const std::string& _value = lua.getLuaValue(-1);
			strMap[_key] = _value;
			lua.pop();
		}

		BindingSpace bindingSpace = BINDINGS_UI;
		if (id == "map")
			bindingSpace = BINDINGS_MAP;
		for (std::map<std::string, std::string>::const_iterator i = strMap.begin(); i != strMap.end(); ++i) {
			if (type == KEYBOARD) {
				map[bindingSpace][i->first] = i->second;
			} else if (type == CONTROLLER) {
				const SDL_GameControllerButton button = SDL_GameControllerGetButtonFromString(i->first.c_str());
				const char *buttonStr = SDL_GameControllerGetStringForButton(button);
				map[bindingSpace][buttonStr] = i->second;
			}
		}

		lua_pop(lua.getState(), 1);
	}
}

void ConfigManager::getBindingMap (LUA& lua, std::map<int, std::string>* map, const char *key, BindingType type)
{
	if (type == CONTROLLER)
		return;

	if (!lua.getGlobalKeyValue(key))
		return;

	while (lua.getNextKeyValue()) {
		const std::string id = lua.getKey();
		if (id.empty()) {
			lua.pop();
			continue;
		}

		lua_pushnil(lua.getState());

		std::map<std::string, std::string> strMap;
		while (lua.getNextKeyValue()) {
			const std::string& _key = lua.getLuaValue(-2);
			const std::string& _value = lua.getLuaValue(-1);
			strMap[_key] = _value;
			lua.pop();
		}

		BindingSpace bindingSpace = BINDINGS_UI;
		if (id == "map")
			bindingSpace = BINDINGS_MAP;
		for (std::map<std::string, std::string>::const_iterator i = strMap.begin(); i != strMap.end(); ++i) {
			if (type == KEYBOARD) {
				map[bindingSpace][mapKey(i->first)] = i->second;
			}
		}

		lua_pop(lua.getState(), 1);
	}
}

void ConfigManager::autoComplete (const std::string& input, std::vector<std::string>& matches)
{
	const std::string tmp(input + "*");
	for (ConfigVarsMapConstIter i = _configVars.begin(); i != _configVars.end(); ++i) {
		if (!string::matches(tmp, i->first))
			continue;
		matches.push_back(i->first);
	}
}

void ConfigManager::listConfigVariables (const ICommand::Args& args)
{
	if (!args.empty()) {
		const std::string tmp(args[0] + "*");
		for (ConfigVarsMapConstIter i = _configVars.begin(); i != _configVars.end(); ++i) {
			if (!string::matches(tmp, i->first))
				continue;
			const ConfigVarPtr& c = i->second;
			Log::info(LOG_COMMON, "%s %s", c->getName().c_str(), c->getValue().c_str());
		}
	} else {
		for (ConfigVarsMapConstIter i = _configVars.begin(); i != _configVars.end(); ++i) {
			const ConfigVarPtr& c = i->second;
			Log::info(LOG_COMMON, "%s %s", c->getName().c_str(), c->getValue().c_str());
		}
	}
}

void ConfigManager::setLogLevel (const ICommand::Args& args)
{
	if (args.size() != 1)
		return;
	const int max = static_cast<int>(LogLevel::LEVEL_MAX);
	for (int i = 0; i < max; ++i) {
		if (args[0] == LogLevels[i].logLevelStr) {
			SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, LogLevels[i].sdlLevel);
			_logLevel = LogLevels[i].logLevel;
			Log::info(LOG_COMMON, "Changing log level to %s", args[0].c_str());
			return;
		}
	}
	Log::error(LOG_COMMON, "Failed to change the level to %s", args[0].c_str());
}

void ConfigManager::setConfig (const ICommand::Args& args)
{
	if (args.size() != 2) {
		Log::error(LOG_COMMON, "parameters: the config key");
		return;
	}

	Log::info(LOG_COMMON, "set %s to %s", args[0].c_str(), args[1].c_str());

	ConfigVarPtr p = getConfigVar(args[0]);
	p->setValue(args[1]);
}

int ConfigManager::mapModifier (const std::string& name)
{
	const std::string converted = string::replaceAll(name, "_", " ");
	const SDL_Keycode keycode = SDL_GetKeyFromName(converted.c_str());
	if (keycode & KMOD_SHIFT) {
		return KMOD_SHIFT;
	} else if (keycode & KMOD_ALT) {
		return KMOD_ALT;
	} else if (keycode & KMOD_CTRL) {
		return KMOD_CTRL;
	}
	return KMOD_NONE;
}

int ConfigManager::mapKey (const std::string& name)
{
	const std::string converted = string::replaceAll(name, "_", " ");
	const SDL_Keycode keycode = SDL_GetKeyFromName(converted.c_str());
	if (keycode == 0)
		return -1;
	return keycode;
}

void ConfigManager::shutdown ()
{
	Log::info(LOG_COMMON, "shutdown config manager");
	_persister->save(_configVars);
	_configVars.clear();
	_bindingSpaceListener = nullptr;
}

ConfigVarPtr ConfigManager::getConfigVar (const std::string& name, const std::string& value, bool create, unsigned int flags)
{
	ConfigVarsMapIter i = _configVars.find(name);
	if (i == _configVars.end()) {
		if (!create)
			return ConfigVarPtr();
		std::string v = _persister->getValue(name);
		if (v.empty())
			v = value;
		ConfigVarPtr p(new ConfigVar(name, v, flags));
		_configVars[name] = p;
		return p;
	}
	return i->second;
}

ConfigVarPtr ConfigManager::getConfigValue (KeyValueMap &map, const std::string& name, const std::string& defaultValue, unsigned int flags)
{
	// we need this no-persist check here, because this wasn't available in every version
	const std::string savedValue = (flags & CV_NOPERSIST) != 0 ? "" : _persister->getValue(name);
	std::string val = savedValue;
	if (val.empty()) {
		KeyValueMap::iterator i = map.find(name);
		if (i != map.end()) {
			val = i->second;
			map.erase(i);
		}
	}
	if (val.empty())
		val = defaultValue;

	if (!savedValue.empty())
		Log::info(LOG_COMMON, "use stored value '%s' for key '%s'", val.c_str(), name.c_str());
	else
		Log::info(LOG_COMMON, "use value '%s' for key '%s'", val.c_str(), name.c_str());
	const ConfigVarPtr& p = getConfigVar(name, val, true, flags);
	return p;
}

int ConfigManager::increaseCounter (const std::string& counterId)
{
	const ConfigVarPtr& var = getConfigVar(counterId, "0", true);
	int current = var->getIntValue();
	++current;
	var->setValue(current);
	return current;
}

bool ConfigManager::isModeSelected () const
{
	const std::string& mode = _mode->getValue();
	return mode == "hard" || mode == "easy";
}
