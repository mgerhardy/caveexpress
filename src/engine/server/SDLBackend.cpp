#include "SDLBackend.h"
#include "engine/common/Logger.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/String.h"
#include "engine/client/SDLFrontend.h"
#ifdef SDL_VIDEO_OPENGL
#include "engine/client/GLFrontend.h"
#endif
#include "engine/client/ClientConsole.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/FileSystem.h"
#include "engine/server/commands/CmdQuit.h"
#include "engine/common/ConsoleFrontend.h"
#include "engine/common/Commands.h"
#include "engine/common/Singleton.h"
#include "engine/GameRegistry.h"
#include "engine/common/network/messages/PingMessage.h"
#include "engine/common/MapManager.h"
#include "engine/client/entities/ClientEntityFactory.h"
#include <SDL.h>
#include <iostream>
#include <time.h>

static SDLBackend *INSTANCE;
#ifdef EMSCRIPTEN
#include <emscripten.h>

static void runFrameEmscripten() {
	if (!INSTANCE->isRunning()) {
		Config.shutdown();
		System.track("SDLBackend", "shutdown");
		info(LOG_BACKEND, "shut down the main loop");
		emscripten_cancel_main_loop();
		return;
	}

	INSTANCE->runFrame();
}
#endif

SDLBackend::SDLBackend () :
		_dedicated(false), _running(true), _frontend(nullptr)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetEventFilter(handleAppEvents, this);
#endif

	Commands.registerCommand(CMD_SCREENSHOT, bind(SDLBackend, screenShot));
	Commands.registerCommand(CMD_MAP_START, bind(SDLBackend, loadMap))->setCompleter(loadMapCompleter);
	Commands.registerCommand(CMD_STATUS, bind(SDLBackend, status));
	_eventHandler.registerObserver(this);
	_keys.clear();
	_joystickButtons.clear();
	_controllerButtons.clear();
}

SDLBackend::~SDLBackend ()
{
	info(LOG_BACKEND, "shutdown backend");
	_eventHandler.removeObserver(this);

	Commands.removeCommand(CMD_SCREENSHOT);
	Commands.removeCommand(CMD_MAP_START);
	Commands.removeCommand(CMD_QUIT);

	if (_frontend) {
		delete _frontend;
		_frontend = nullptr;
	}

	SDL_Quit();
}

int SDLBackend::init (int argc, char **argv)
{
	for (int i = 0; i < argc; i++) {
		if (!strcmp("-server", argv[i])) {
			_dedicated = true;
			break;
		}
	}

	srand(time(nullptr));

#ifdef EMSCRIPTEN
	FS.get().init("http", "127.0.0.1/");
#else
	FS.get().init("file", "");
#endif

	Config.get().init(this, argc, argv);
	Commands.registerCommand(CMD_QUIT, new CmdQuit());

	if (_dedicated) {
		_frontend = new ConsoleFrontend(_console);
	} else {
		const ConfigVarPtr c = Config.getConfigVar("frontend", "sdl", true);
		info(LOG_CLIENT, "Use frontend " + c->getValue());
		SharedPtr<IConsole> clientConsole(new ClientConsole());
#ifdef SDL_VIDEO_OPENGL
		if (c->getValue() == "opengl")
			_frontend = new GLFrontend(clientConsole);
		else
#endif
			_frontend = new SDLFrontend(clientConsole);
	}

	srand(SDL_GetTicks());

	if (!SDL_WasInit(SDL_INIT_TIMER)) {
		if (SDL_Init(SDL_INIT_TIMER) == -1) {
			info(LOG_CLIENT, "Failed to initialize the timers: " + std::string(SDL_GetError()));
		}
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
#ifdef DEBUG
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
	//SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
#endif
#endif

	const int frontendInit = _frontend->init(Config.getWidth(), Config.getHeight(), Config.isFullscreen(), _eventHandler);
	_serviceProvider.init(_frontend);
	_serviceProvider.updateNetwork(_dedicated);
	SpriteDefinition::get().init(_serviceProvider.getTextureDefinition());
	info(LOG_BACKEND, "initialize game data");
	Singleton<GameRegistry>::getInstance().getGame()->init(_frontend, _serviceProvider);

	// precache some stuff here
	Singleton<ClientEntityRegistry>::getInstance().initSounds();

	info(LOG_BACKEND, "initialize frontend");
	_frontend->initUI(_serviceProvider);

	handleCommandLineArguments(argc, argv);

	info(LOG_BACKEND, "frontend start");
	_frontend->onStart();

	return frontendInit;
}

bool SDLBackend::isRunning () const
{
	return _running;
}

void SDLBackend::handleEvent (SDL_Event &event)
{
	// TODO: not working on linux
#if 0
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_ALT)) {
			const int isFullscreen = _frontend->isFullscreen();
			_frontend->setFullscreen(!isFullscreen);
		}
	}
#endif

	switch (event.type) {
	case SDL_QUIT:
		info(LOG_BACKEND, "received quit event");
		if (!System.quit()) {
			_running = false;
		}
		break;
	default: {
		if (!_running)
			return;
		const bool running = _eventHandler.handleEvent(event);
		// don't overwrite a SDL_QUIT event
		if (!running)
			_running = running;
		break;
	}
	}

	if (!_running) {
		info(LOG_BACKEND, "Quitting the game");

		_frontend->shutdown();
		Singleton<GameRegistry>::getInstance().getGame()->shutdown();
	}
}

void SDLBackend::handleCommandLineArguments (int argc, char **argv)
{
	info(LOG_CLIENT, "execute commandline");
	ICommand::Args args;
	std::string command;
	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-server"))
			continue;
		if (argv[i][0] == '-') {
			if (!command.empty()) {
				Commands.executeCommand(command, args);
			}
			args.clear();
			command = &argv[i][1];
			continue;
		}
		args.push_back(argv[i]);
	}
	if (!command.empty()) {
		Commands.executeCommand(command, args);
	}
	info(LOG_CLIENT, "commandline handled");
}

void SDLBackend::frameCallback (void *userdata)
{
	SDLBackend *backend = static_cast<SDLBackend*>(userdata);
	backend->runFrame();
}

int SDLBackend::handleAppEvents (void *userdata, SDL_Event *event)
{
	SDLBackend* backend = static_cast<SDLBackend*>(userdata);
	if (backend->_eventHandler.handleAppEvent(*event))
		return 0;
	return 1;
}

void SDLBackend::resetKeyStates ()
{
	KeyMap keyMap = _keys;
	JoystickSet joystickSet = _joystickButtons;
	ControllerSet controllerSet = _controllerButtons;

	for (KeyMapConstIter it = keyMap.begin(); it != keyMap.end(); ++it) {
		onKeyRelease(it->first);
	}
	for (JoystickSet::const_iterator it = joystickSet.begin(); it != joystickSet.end(); ++it) {
		onJoystickButtonRelease(*it);
	}
	for (ControllerSet::const_iterator it = controllerSet.begin(); it != controllerSet.end(); ++it) {
		onControllerButtonRelease(*it);
	}

	_keys.clear();
	_joystickButtons.clear();
	_controllerButtons.clear();
}

void SDLBackend::runFrame ()
{
	SDL_Event event;
	static uint32_t lastFrame = SDL_GetTicks() - 2;

	const uint32_t now = SDL_GetTicks();
	const uint32_t deltaTime = std::min(500U, now - lastFrame);
	lastFrame = now;

	while (SDL_PollEvent(&event)) {
		handleEvent(event);
		if (!_running)
			return;
	}

	// fake a key press event for held down keys
	for (KeyMapConstIter it = _keys.begin(); it != _keys.end(); ++it) {
		onKeyPress(it->first, it->second);
	}
	for (JoystickSet::const_iterator it = _joystickButtons.begin(); it != _joystickButtons.end(); ++it) {
		onJoystickButtonPress(*it);
	}
	for (ControllerSet::const_iterator it = _controllerButtons.begin(); it != _controllerButtons.end(); ++it) {
		onControllerButtonPress(*it);
	}

	_serviceProvider.getNetwork().update(deltaTime);

	Singleton<GameRegistry>::getInstance().getGame()->update(deltaTime);
	_frontend->update(deltaTime);
	_frontend->render();
	System.tick(deltaTime);
}

void SDLBackend::mainLoop (int argc, char **argv)
{
	System.track("SDLBackend", "start");

	if (init(argc, argv) == -1) {
		System.exit("Initialization error", EXIT_FAILURE);
	}

	info(LOG_BACKEND, "initialization done");

	if (_frontend->setFrameCallback(2, frameCallback, this))
		return;

	INSTANCE = this;
#ifdef EMSCRIPTEN
	emscripten_set_main_loop(runFrameEmscripten, 0, 1);
#else
	while (_running) {
		runFrame();

		SDL_Delay(1);
	}

	Config.shutdown();
	_serviceProvider.getNetwork().shutdown();

#endif
	System.track("SDLBackend", "shutdown");
}

bool SDLBackend::onKeyRelease (int32_t key)
{
	if (_frontend->handlesInput())
		return false;
	const std::string command = Config.getKeyBinding(key);
	if (command.empty()) {
		return false;
	}

	if (command[0] != '+') {
		return false;
	}

	if (_keys.erase(key) > 0) {
		Commands.executeCommandLine(command + " -");
	}

	return true;
}

bool SDLBackend::onKeyPress (int32_t key, int16_t modifier)
{
	if (_frontend->handlesInput())
		return false;
	const std::string command = Config.getKeyBinding(key);
	if (command.empty()) {
		return false;
	}

	const int mod = Config.getKeyModifier(key);
	if (mod != KMOD_NONE && !(modifier & mod)) {
		return false;
	}

	if (command[0] == '+') {
		if (_keys.find(key) == _keys.end()) {
			_keys[key] = modifier;
			Commands.executeCommandLine(command);
			return true;
		}
	} else {
		Commands.executeCommandLine(command);
		return true;
	}

	return false;
}

void SDLBackend::onJoystickButtonPress (uint8_t button)
{
	if (_frontend->handlesInput())
		return;
	const std::string command = Config.getJoystickBinding(button);
	if (command.empty()) {
		info(LOG_SERVER, String::format("no binding for joystick button %i", (int)button));
		return;
	}

	if (command[0] == '+') {
		if (_joystickButtons.find(button) == _joystickButtons.end()) {
			_joystickButtons.insert(button);
			Commands.executeCommandLine(command);
		}
	} else {
		Commands.executeCommandLine(command);
	}
}

void SDLBackend::onJoystickButtonRelease (uint8_t button)
{
	if (_frontend->handlesInput())
		return;
	const std::string command = Config.getJoystickBinding(button);
	if (command.empty()) {
		return;
	}

	if (command[0] != '+') {
		return;
	}

	if (_joystickButtons.erase(button) > 0) {
		Commands.executeCommandLine(command + " -");
	}
}

void SDLBackend::onControllerButtonPress (const std::string& button)
{
	if (_frontend->handlesInput())
		return;
	const std::string command = Config.getControllerBinding(button);
	if (command.empty()) {
		info(LOG_SERVER, "no binding for controller button " + button);
		return;
	}

	if (command[0] == '+') {
		if (_controllerButtons.find(button) == _controllerButtons.end()) {
			Commands.executeCommandLine(command);
			_controllerButtons.insert(button);
		}
	} else {
		Commands.executeCommandLine(command);
	}
}

void SDLBackend::onControllerButtonRelease (const std::string& button)
{
	if (_frontend->handlesInput())
		return;
	const std::string command = Config.getControllerBinding(button);
	if (command.empty()) {
		return;
	}

	if (command[0] != '+') {
		return;
	}

	if (_controllerButtons.erase(button) > 0) {
		Commands.executeCommandLine(command + " -");
	}
}

void SDLBackend::screenShot (const std::string& argument)
{
	std::string name = argument;
	if (name.empty()) {
		name = Singleton<GameRegistry>::getInstance().getGame()->getMapName();
		if (name.empty())
			name = "screenshot";
	}
	_frontend->makeScreenshot(name);
}

void SDLBackend::status ()
{
	const std::string& map = Singleton<GameRegistry>::getInstance().getGame()->getMapName();

	if (map.empty()) {
		info(LOG_SERVER, "no map loaded");
		return;
	}

	info(LOG_SERVER, "map running: " + map);
}

void SDLBackend::loadMapCompleter (const std::string& input, std::vector<std::string>& matches)
{
	const IMapManager::Maps& maps = INSTANCE->_serviceProvider.getMapManager().getMapsByWildcard(input + "*");
	for (IMapManager::Maps::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		matches.push_back(i->first);
	}
}

void SDLBackend::loadMap (const std::string& mapName)
{
	_serviceProvider.getNetwork().closeServer();
	if (Singleton<GameRegistry>::getInstance().getGame()->mapLoad(mapName)) {
		if (!_serviceProvider.getNetwork().openServer(Config.getPort(), this)) {
			error(LOG_BACKEND, "failed to start the server");
			return;
		}
		_frontend->connect();
		return;
	}

	info(LOG_BACKEND, "failed to load map " + mapName);
}

void SDLBackend::onData (ClientId clientId, ByteStream &data)
{
	Singleton<GameRegistry>::getInstance().getGame()->onData(clientId, data);
}

ProtocolMessagePtr SDLBackend::onOOBData (const unsigned char *data)
{
	if (strcmp("discover", reinterpret_cast<const char*>(data)))
		return ProtocolMessagePtr();

	// no map loaded
	const GamePtr& game = Singleton<GameRegistry>::getInstance().getGame();
	const std::string& mapName = game->getMapName();
	if (mapName.empty())
		return ProtocolMessagePtr();

	const ProtocolMessagePtr msg(new PingMessage(Config.getServerName(), mapName, Config.getPort(), game->getPlayers(), MAX_CLIENTS));
	return msg;
}

void SDLBackend::onConnection (ClientId clientId)
{
	info(LOG_SERVER, "connect of client with id " + string::toString(static_cast<int>(clientId)));
	Singleton<GameRegistry>::getInstance().getGame()->connect(clientId);
}

void SDLBackend::onDisconnect (ClientId clientId)
{
	info(LOG_SERVER, "disconnect of client with id " + string::toString(static_cast<int>(clientId)));
	const GamePtr& game = Singleton<GameRegistry>::getInstance().getGame();
	if (game->disconnect(clientId) == 0) {
		if (_dedicated)
			game->mapReload();
		else
			game->mapShutdown();
	}
}
