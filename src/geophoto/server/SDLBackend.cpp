#include "SDLBackend.h"
#include "shared/Logger.h"
#include "shared/network/INetwork.h"
#include "common/String.h"
#include "client/SDLFrontend.h"
#ifdef SDL_VIDEO_OPENGL
#include "client/GLFrontend.h"
#endif
#include "client/ClientConsole.h"
#include "shared/ConfigManager.h"
#include "shared/FileSystem.h"
#include "shared/SpriteDefinition.h"
#include "shared/CommandSystem.h"
#include "server/commands/CmdQuit.h"
#include "shared/network/ProtocolHandlerRegistry.h"
#include "shared/network/messages/PingMessage.h"
#include <SDL.h>
#include <iostream>
#include <time.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>

static SDLBackend *INSTANCE;

static void runFrameEmscripten() {
	if (!INSTANCE->isRunning()) {
		Config.shutdown();
		info(LOG_BACKEND, "shut down the main loop");
		return;
	}

	INSTANCE->runFrame();
}
#endif

SDLBackend::SDLBackend () :
		_running(true), _frontend(nullptr)
{
#ifndef EMSCRIPTEN
	SDL_SetEventFilter(handleAppEvents, this);
#endif

	_eventHandler.registerObserver(this);
	_keys.clear();
}

SDLBackend::~SDLBackend ()
{
	info(LOG_BACKEND, "shutdown backend");
	_eventHandler.removeObserver(this);

	Commands.removeCommand(CMD_QUIT);

	if (_frontend) {
		delete _frontend;
		_frontend = nullptr;
	}

	SDL_Quit();
}

int SDLBackend::init (int argc, char **argv)
{
	//FS.get().init("http", "127.0.0.1/");
	FS.get().init("file", "");

	Config.get().init();
	Commands.registerCommand(CMD_QUIT, new CmdQuit(_running));

	handleCommandLineArguments(argc, argv, true);

	const ConfigVarPtr c = Config.getConfigVar("frontend", "sdl", true);
	SharedPtr<IConsole> clientConsole(new ClientConsole());
	info(LOG_CLIENT, "Use frontend " + c->getValue());
#ifdef SDL_VIDEO_OPENGL
	if (c->getValue() == "opengl")
		_frontend = new GLFrontend(clientConsole);
	else
#endif
		_frontend = new SDLFrontend(clientConsole);

	srand(SDL_GetTicks());

	if (!SDL_WasInit(SDL_INIT_TIMER)) {
		if (SDL_Init(SDL_INIT_TIMER) == -1) {
			return -1;
		}
	}

	const int frontendInit = _frontend->init(Config.getWidth(), Config.getHeight(), Config.isFullscreen(), _eventHandler);
	_serviceProvider.init(_frontend);
	SpriteDefinition::get().init(_serviceProvider);
	info(LOG_BACKEND, "initialize frontend");
	_frontend->initUI(_serviceProvider);

	handleCommandLineArguments(argc, argv, false);

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
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_ALT)) {
			const int isFullscreen = _frontend->isFullscreen();
			_frontend->setFullscreen(!isFullscreen);
		}
	}

	switch (event.type) {
	case SDL_QUIT:
		_running = false;
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
	}
}

void SDLBackend::handleCommandLineArguments (int argc, char **argv, bool handleConfigVars)
{
	info(LOG_CLIENT, "execute commandline");
	ICommand::Args args;
	std::string command;
	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-server"))
			continue;
		if (argv[i][0] == '-') {
			if (!command.empty()) {
				if ((!handleConfigVars && command != CMD_SETVAR) || (handleConfigVars && command == CMD_SETVAR))
					Commands.executeCommand(command, args);
			}
			args.clear();
			command = &argv[i][1];
			continue;
		}
		args.push_back(argv[i]);
	}
	if (!command.empty()) {
		if ((!handleConfigVars && command != CMD_SETVAR) || (handleConfigVars && command == CMD_SETVAR))
			Commands.executeCommand(command, args);
	}
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

void SDLBackend::runFrame ()
{
	SDL_Event event;
	static uint32_t lastFrame = SDL_GetTicks() - 2;

	const uint32_t now = SDL_GetTicks();
	const uint32_t deltaTime = now - lastFrame;
	lastFrame = now;

	while (SDL_PollEvent(&event)) {
		handleEvent(event);
		if (!_running)
			return;
	}

	if (!_keys.empty()) {
		// fake a key press event for held down keys
		for (KeyMapConstIter it = _keys.begin(); it != _keys.end(); ++it) {
			onKeyPress(it->first, it->second);
		}
	}

	_serviceProvider.getNetwork().update(deltaTime);

	_frontend->update(deltaTime);
	_frontend->render();
	System.tick(deltaTime);
}

void SDLBackend::mainLoop (int argc, char **argv)
{
	if (init(argc, argv) == -1) {
		System.exit("Initialization error", EXIT_FAILURE);
	}

	info(LOG_BACKEND, "initialization done");

	if (_frontend->setFrameCallback(2, frameCallback, this))
		return;

#ifdef EMSCRIPTEN
	INSTANCE = this;
	emscripten_set_main_loop(runFrameEmscripten, 0, 1);
#else
	while (_running) {
		runFrame();

		SDL_Delay(1);
	}

	Config.shutdown();
	_serviceProvider.getNetwork().shutdown();
#endif
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

void SDLBackend::onData (ClientId clientId, ByteStream &data)
{
	while (!data.empty()) {
		const ScopedPtr<IProtocolMessage> msg(ProtocolMessageFactory::get().create(data));
		if (!msg) {
			error(LOG_SERVER, "no message for type " + string::toString(static_cast<int>(data.readByte())));
			continue;
		}
		IServerProtocolHandler* handler = ProtocolHandlerRegistry::get().getServerHandler(*msg);
		if (handler == nullptr) {
			error(LOG_SERVER, String::format("no server handler for message type %i", msg->getId()));
			continue;
		}
		handler->execute(clientId, *msg);
	}
}

ProtocolMessagePtr SDLBackend::onOOBData (const unsigned char *data)
{
	if (strcmp("discover", reinterpret_cast<const char*>(data)))
		return ProtocolMessagePtr();

	info(LOG_SERVER, "ping request");
	ProtocolMessagePtr msg(new PingMessage("noname", Config.getPort(), 2));
	return msg;
}

void SDLBackend::onConnection (ClientId clientId)
{
	info(LOG_SERVER, "connect of client with id " + string::toString(static_cast<int>(clientId)));
}

void SDLBackend::onDisconnect (ClientId clientId)
{
	info(LOG_SERVER, "disconnect of client with id " + string::toString(static_cast<int>(clientId)));
}
