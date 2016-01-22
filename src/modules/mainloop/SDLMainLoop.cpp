#include <mainloop/SDLMainLoop.h>
#include "common/Log.h"
#include "network/INetwork.h"
#include "common/String.h"
#include "gfx/SDLFrontend.h"
#ifdef SDL_VIDEO_OPENGL
#include "gfx/GL1Frontend.h"
#endif
#if defined(SDL_VIDEO_OPENGL_ES) || defined(SDL_VIDEO_OPENGL_ES2)
#include "gfx/GL3Frontend.h"
#endif
#include "client/ClientConsole.h"
#include "common/ConfigManager.h"
#include "common/System.h"
#include "common/SpriteDefinition.h"
#include "common/CommandSystem.h"
#include "common/FileSystem.h"
#include "common/ConsoleFrontend.h"
#include "common/Commands.h"
#include "common/Singleton.h"
#include "game/GameRegistry.h"
#include "network/messages/PingMessage.h"
#include "common/MapManager.h"
#include "network/ProtocolHandlerRegistry.h"
#include "client/entities/ClientEntityFactory.h"
#include <SDL.h>
#include <time.h>

static SDLMainLoop *INSTANCE;
#ifdef EMSCRIPTEN
#include <emscripten.h>

static void runFrameEmscripten() {
	if (!INSTANCE->isRunning()) {
		Config.shutdown();
		System.track("step", "sdl backend shutdown");
		Log::info(LOG_SERVER, "shut down the main loop");
		emscripten_cancel_main_loop();
		return;
	}

	// TODO: fps limit
	INSTANCE->runFrame();
}
#endif

SDLMainLoop::SDLMainLoop () :
		_dedicated(false), _running(true), _initState(InitState::INITSTATE_CONFIG), _argc(0), _argv(nullptr), _frontend(nullptr)
{
	SDL_SetEventFilter(handleAppEvents, this);

	Commands.registerCommandString(CMD_SCREENSHOT, bindFunction(SDLMainLoop::screenShot));
	Commands.registerCommandString(CMD_MAP_START, bindFunction(SDLMainLoop::loadMap))->setCompleter(loadMapCompleter);
	Commands.registerCommandVoid(CMD_STATUS, bindFunctionVoid(SDLMainLoop::status));
	_eventHandler.registerObserver(this);
	_keys.clear();
	_controllerButtons.clear();
}

SDLMainLoop::~SDLMainLoop ()
{
	Log::info(LOG_SERVER, "shutdown backend");
	_eventHandler.removeObserver(this);

	Commands.removeCommand(CMD_SCREENSHOT);
	Commands.removeCommand(CMD_MAP_START);
	Commands.removeCommand(CMD_QUIT);
	Commands.removeCommand(CMD_STATUS);

	if (_frontend) {
		delete _frontend;
		_frontend = nullptr;
	}

	SDL_Quit();
}

bool SDLMainLoop::isRunning () const
{
	return _running;
}

void SDLMainLoop::handleEvent (SDL_Event &event)
{
	if (event.type == SDL_KEYDOWN) {
		const SDL_Keycode sym = event.key.keysym.sym;
		const uint16_t mod = event.key.keysym.mod;
		if (sym == SDLK_RETURN && (mod & KMOD_ALT)) {
			const int isFullscreen = _frontend->isFullscreen();
			_frontend->setFullscreen(!isFullscreen);
		} else if (sym == SDLK_g && (mod & KMOD_CTRL)) {
			_frontend->toggleGrabMouse();
		} else if (sym == SDLK_r && (mod & KMOD_CTRL)) {
			SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode() ? SDL_TRUE : SDL_FALSE);
		}
	}

	switch (event.type) {
	case SDL_QUIT:
		Log::info(LOG_SERVER, "received quit event");
		Config.shutdown();
		if (!System.quit()) {
			_running = false;
		}
		break;
	default: {
		if (!_running)
			return;
		Log::trace(LOG_SERVER, "received event of type %i", event.type);
		const bool running = _eventHandler.handleEvent(event);
		// don't overwrite a SDL_QUIT event
		if (!running)
			_running = running;
		break;
	}
	}

	if (!_running) {
		Log::info(LOG_SERVER, "Quitting the game");

		_frontend->shutdown();
		getGame()->shutdown();
	}
}

void SDLMainLoop::handleCommandLineArguments (int argc, char **argv)
{
	Log::info(LOG_SERVER, "execute commandline");
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
	Log::info(LOG_SERVER, "commandline handled");
}

void SDLMainLoop::frameCallback (void *userdata)
{
	SDLMainLoop *backend = static_cast<SDLMainLoop*>(userdata);
	backend->runFrame();
}

int SDLMainLoop::handleAppEvents (void *userdata, SDL_Event *event)
{
	SDLMainLoop* backend = static_cast<SDLMainLoop*>(userdata);
	if (backend->_eventHandler.handleAppEvent(*event))
		return 0;
	return 1;
}

void SDLMainLoop::resetKeyStates ()
{
	KeyMap keyMap = _keys;
	ControllerSet controllerSet = _controllerButtons;

	for (auto it : keyMap) {
		onKeyRelease(it.first);
	}
	for (const ControllerButton& button : controllerSet) {
		onControllerButtonRelease(button.button, button.id);
	}

	_keys.clear();
	_controllerButtons.clear();
}

bool SDLMainLoop::handleInit() {
	switch (_initState) {
	case InitState::INITSTATE_CONFIG:
		Log::info(LOG_SERVER, "init config");
		Config.init(this, _argc, _argv);
		Commands.registerCommandVoid(CMD_QUIT, [] () {
			SDL_Event event;
			event.type = SDL_QUIT;
			SDL_PushEvent(&event);
		});
		_initState = InitState::INITSTATE_FRONTEND;
		break;
	case InitState::INITSTATE_FRONTEND:
		Log::info(LOG_SERVER, "init frontend creation");
		if (_dedicated) {
			_frontend = new ConsoleFrontend(_console);
		} else {
			const ConfigVarPtr c = Config.getConfigVar("frontend", "sdl", true);
			Log::info(LOG_SERVER, "Use frontend %s", c->getValue().c_str());
			std::shared_ptr<IConsole> clientConsole(new ClientConsole());
#ifdef SDL_VIDEO_OPENGL
			if (c->getValue() == "opengl")
				_frontend = new GL1Frontend(clientConsole);
			else
#endif
#if defined(SDL_VIDEO_OPENGL_ES) || defined(SDL_VIDEO_OPENGL_ES2)
			if (c->getValue() == "opengl3")
				_frontend = new GL3Frontend(clientConsole);
			else
#endif
				_frontend = new SDLFrontend(clientConsole);
		}
		_initState = InitState::INITSTATE_SDL;
		break;
	case InitState::INITSTATE_SDL:
		Log::info(LOG_SERVER, "init sdl");

		if (!SDL_WasInit(SDL_INIT_TIMER)) {
			if (SDL_Init(SDL_INIT_TIMER) == -1) {
				Log::info(LOG_SERVER, "Failed to initialize the timers: %s", SDL_GetError());
			}
		}

		srand(SDL_GetTicks());

#ifdef DEBUG
		//SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
		//SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
#endif
		_initState = InitState::INITSTATE_FRONTENDINIT;
		break;
	case InitState::INITSTATE_FRONTENDINIT: {
		Log::info(LOG_SERVER, "init frontend");
		const int frontendInit = _frontend->init(Config.getWidth(), Config.getHeight(), Config.isFullscreen(), _eventHandler);
		if (frontendInit == -1) {
			Log::error(LOG_SERVER, "Failed to initialize the frontend");
			_initState = InitState::INITSTATE_ERROR;
			break;
		}
		_initState = InitState::INITSTATE_SERVICEPROVIDER;
		break;
	}
	case InitState::INITSTATE_SERVICEPROVIDER:
		Log::info(LOG_SERVER, "init the serviceprovider");
		_serviceProvider.init(_frontend);
		_serviceProvider.updateNetwork(_dedicated);
		_initState = InitState::INITSTATE_SPRITE;
		break;
	case InitState::INITSTATE_SPRITE:
		Log::info(LOG_SERVER, "init sprites");
		SpriteDefinition::get().init(_serviceProvider.getTextureDefinition());
		_initState = InitState::INITSTATE_GAME;
		break;
	case InitState::INITSTATE_GAME:
		Log::info(LOG_SERVER, "init game data %s", getGame()->getName().c_str());
		getGame()->init(_frontend, _serviceProvider);
		_initState = InitState::INITSTATE_SOUNDS;
		break;
	case InitState::INITSTATE_SOUNDS:
		Log::info(LOG_SERVER, "init sounds");
		// precache some stuff here
		Singleton<ClientEntityRegistry>::getInstance().initSounds();
		_initState = InitState::INITSTATE_UI;
		break;
	case InitState::INITSTATE_UI:
		Log::info(LOG_SERVER, "init ui");
		_frontend->initUI(_serviceProvider);
		_initState = InitState::INITSTATE_START;
		break;
	case InitState::INITSTATE_START:
		handleCommandLineArguments(_argc, _argv);
		Log::info(LOG_SERVER, "frontend start");
		_frontend->onStart();
		_initState = InitState::INITSTATE_DONE;

		Log::info(LOG_SERVER, "initialization done");

		break;
	case InitState::INITSTATE_ERROR:
		_running = false;
		break;
	case InitState::INITSTATE_DONE:
		return false;
	}
	Log::info(LOG_SERVER, "next init state: %i", (int)_initState);
	return true;
}

void SDLMainLoop::runFrame ()
{
	if (handleInit())
		return;

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
	for (auto it : _keys) {
		onKeyPress(it.first, it.second);
	}
	for (const ControllerButton& button : _controllerButtons) {
		onControllerButtonPress(button.button, button.id);
	}

	_serviceProvider.getNetwork().update(deltaTime);

	getGame()->update(deltaTime);
	_frontend->update(deltaTime);
	_frontend->render();
	System.tick(deltaTime);
}

void SDLMainLoop::mainLoop (int argc, char **argv)
{
	System.track("step", "sdl backend start");
	_argc = argc;
	_argv = argv;

	for (int i = 0; i < _argc; i++) {
		if (!strcmp("-server", _argv[i])) {
			_dedicated = true;
			break;
		}
	}

	srand(time(nullptr));

	INSTANCE = this;
#ifdef EMSCRIPTEN
	emscripten_set_main_loop(runFrameEmscripten, 0, 1);
	return;
#endif

	while (_running) {
		if (_initState == InitState::INITSTATE_DONE) {
			break;
		}
		runFrame();
	}

	if (_frontend != nullptr && _frontend->setFrameCallback(2, frameCallback, this))
		return;

	const ConfigVarPtr& fpsLimit = Config.getConfigVar("fpslimit", "60.0", true);
	Log::info(LOG_SERVER, "Run the game at %f frames per second", fpsLimit->getFloatValue());
	double nextFrame = static_cast<double>(SDL_GetTicks());
	while (_running) {
		const double tick = static_cast<double>(SDL_GetTicks());
		if (nextFrame > tick) {
			runFrame();
		}
		const int32_t delay = static_cast<int32_t>(nextFrame - tick);
		if (delay > 0) {
			SDL_Delay(delay);
		}
		const float fpsCap = fpsLimit->getFloatValue();
		nextFrame += 1000.0 / fpsCap;
	}

	_frontend->shutdown();
	_serviceProvider.shutdown();
	Config.shutdown();

	System.track("step", "sdl backend shutdown");
}

bool SDLMainLoop::onKeyRelease (int32_t key)
{
	if (_frontend->handlesInput())
		return false;
	const std::string& command = Config.getKeyBinding(key);
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

bool SDLMainLoop::onKeyPress (int32_t key, int16_t modifier)
{
	if (_frontend->handlesInput())
		return false;
	Log::debug(LOG_SERVER, "Backend received key press event for key %i with modifier %i", key, modifier);
	const std::string& command = Config.getKeyBinding(key);
	if (command.empty()) {
		Log::debug(LOG_SERVER, "No command found that is bound to key %i", key);
		return false;
	}

	const int mod = Config.getKeyModifier(key);
	const int whiteListModifier = modifier & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI);
	if (mod == KMOD_NONE && whiteListModifier != 0) {
		Log::debug(LOG_SERVER, "Modifiers for binding doesn't match for key %i and command %s (bound: none, pressed: %i)",
				key, command.c_str(), whiteListModifier);
		return false;
	}
	if (mod != KMOD_NONE && !(whiteListModifier & mod)) {
		Log::debug(LOG_SERVER, "Modifiers for binding doesn't match for key %i and command %s (bound: %i, pressed: %i)",
				key, command.c_str(), mod, whiteListModifier);
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

	Log::debug(LOG_SERVER, "Could not execute any key bindings for key %i with modifier %i", key, modifier);
	return false;
}

void SDLMainLoop::onControllerButtonPress (const std::string& button, uint32_t id)
{
	if (_frontend->handlesInput())
		return;
	const std::string command = Config.getControllerBinding(button);
	if (command.empty()) {
		Log::info(LOG_SERVER, "no binding for controller button %s", button.c_str());
		return;
	}

	if (command[0] == '+') {
		const ControllerButton b = { id, button };
		if (_controllerButtons.find(b) == _controllerButtons.end()) {
			Commands.executeCommandLine(command);
			_controllerButtons.insert(b);
		}
	} else {
		Commands.executeCommandLine(command);
	}
}

void SDLMainLoop::onControllerButtonRelease (const std::string& button, uint32_t id)
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

	const ControllerButton b = { id, button };
	if (_controllerButtons.erase(b) > 0) {
		Commands.executeCommandLine(command + " -");
	}
}

void SDLMainLoop::screenShot (const std::string& argument)
{
	std::string name = argument;
	if (name.empty()) {
		name = getGame()->getMapName();
		if (name.empty())
			name = "screenshot";
	}
	_frontend->makeScreenshot(name);
}

void SDLMainLoop::status ()
{
	const std::string& map = getGame()->getMapName();

	if (map.empty()) {
		Log::info(LOG_SERVER, "no map loaded");
		return;
	}

	Log::info(LOG_SERVER, "map running: %s", map.c_str());
}

void SDLMainLoop::loadMapCompleter (const std::string& input, std::vector<std::string>& matches)
{
	const IMapManager::Maps& maps = INSTANCE->_serviceProvider.getMapManager().getMapsByWildcard(input + "*");
	for (IMapManager::Maps::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		matches.push_back(i->first);
	}
}

void SDLMainLoop::loadMap (const std::string& mapName)
{
	_serviceProvider.getNetwork().closeServer();
	if (getGame()->mapLoad(mapName)) {
		if (!_serviceProvider.getNetwork().openServer(Config.getPort(), this)) {
			Log::error(LOG_SERVER, "failed to start the server");
			return;
		}
		Log::info(LOG_SERVER, "connect to own server");
		_frontend->connect();
		return;
	}

	Log::info(LOG_SERVER, "failed to load map %s", mapName.c_str());
}

void SDLMainLoop::onData (ClientId clientId, ByteStream &data)
{
	ProtocolMessageFactory& factory = ProtocolMessageFactory::get();
	while (factory.isNewMessageAvailable(data)) {
		// remove the size from the stream
		data.readShort();
		const IProtocolMessage* msg(factory.createMsg(data));
		if (!msg) {
			Log::error(LOG_SERVER, "no message for type '%i'", static_cast<int>(data.readByte()));
			continue;
		}
		IServerProtocolHandler* handler = ProtocolHandlerRegistry::get().getServerHandler(*msg);
		if (handler == nullptr) {
			Log::error(LOG_SERVER, "no server handler for message type %i", msg->getId());
			continue;
		}
		handler->execute(clientId, *msg);
	}
}

ProtocolMessagePtr SDLMainLoop::onOOBData (const unsigned char *data)
{
	if (strcmp("discover", reinterpret_cast<const char*>(data)))
		return ProtocolMessagePtr();

	// no map loaded
	const GamePtr& game = getGame();
	const std::string& mapName = game->getMapName();
	if (mapName.empty())
		return ProtocolMessagePtr();

	const ProtocolMessagePtr msg(new PingMessage(Config.getServerName(), mapName, Config.getPort(), game->getPlayers(), game->getMaxClients()));
	return msg;
}

void SDLMainLoop::onConnection (ClientId clientId)
{
	Log::info(LOG_SERVER, "connect of client with id %i", static_cast<int>(clientId));
	getGame()->connect(clientId);
}

void SDLMainLoop::onDisconnect (ClientId clientId)
{
	Log::info(LOG_SERVER, "disconnect of client with id %i", static_cast<int>(clientId));
	const GamePtr& game = getGame();
	if (game->disconnect(clientId) == 0) {
		if (_dedicated)
			game->mapReload();
		else
			game->mapShutdown();
	}
}
