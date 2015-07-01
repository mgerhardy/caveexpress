#pragma once

#include "common/NonCopyable.h"
#include "common/IFrontend.h"
#include "common/EventHandler.h"
#include "common/IEventObserver.h"
#include "common/IBindingSpaceListener.h"
#include "common/TextConsole.h"
#include "service/ServiceProvider.h"
#include "common/Singleton.h"
#include "GameRegistry.h"
#include "network/IServerCallback.h"
#include <map>
#include <set>

enum class InitState {
	INITSTATE_CONFIG,
	INITSTATE_FRONTEND,
	INITSTATE_SDL,
	INITSTATE_FRONTENDINIT,
	INITSTATE_SERVICEPROVIDER,
	INITSTATE_SPRITE,
	INITSTATE_GAME,
	INITSTATE_SOUNDS,
	INITSTATE_UI,
	INITSTATE_START,

	INITSTATE_ERROR,
	INITSTATE_DONE
};

class SDLBackend: public NonCopyable, public IEventObserver, public IServerCallback, public IBindingSpaceListener {
private:
	bool _dedicated;
	bool _running;
	InitState _initState;
	int _argc;
	char **_argv;
	IFrontend *_frontend;
	EventHandler _eventHandler;
	typedef std::map<int, int16_t> KeyMap;
	typedef KeyMap::const_iterator KeyMapConstIter;
	typedef KeyMap::iterator KeyMapIter;
	KeyMap _keys;
	typedef std::set<uint8_t> JoystickSet;
	JoystickSet _joystickButtons;
	typedef std::set<std::string> ControllerSet;
	ControllerSet _controllerButtons;
	ServiceProvider _serviceProvider;
	TextConsole _console;

	bool handleInit();
	void handleEvent (SDL_Event &event);
	void render ();
	void update (float deltaTime);
	void handleCommandLineArguments (int argc, char **argv);
	void screenShot (const std::string& argument);
	void status ();

	static void frameCallback (void *userdata);
	static int handleAppEvents (void *userdata, SDL_Event *event);

	static void loadMapCompleter (const std::string& input, std::vector<std::string>& matches);

	inline GamePtr getGame() const {
		return Singleton<GameRegistry>::getInstance().getGame();
	}
public:
	SDLBackend ();
	virtual ~SDLBackend ();

	bool isRunning () const;
	void mainLoop (int argc, char **argv);

	void runFrame ();
	void loadMap (const std::string& mapName);

	void resetKeyStates () override;

	// IEventObserver
	bool onKeyRelease (int32_t key) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;
	void onJoystickButtonPress (uint8_t button) override;
	void onJoystickButtonRelease (uint8_t button) override;
	void onControllerButtonPress (const std::string& button) override;
	void onControllerButtonRelease (const std::string& button) override;

	// IServerCallback
	void onData (ClientId clientId, ByteStream &data) override;
	ProtocolMessagePtr onOOBData (const unsigned char *data) override;
	void onConnection (ClientId clientId) override;
	void onDisconnect (ClientId clientId) override;
};
