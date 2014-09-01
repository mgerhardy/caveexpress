#pragma once

#include "common/NonCopyable.h"
#include "shared/IFrontend.h"
#include "shared/EventHandler.h"
#include "shared/IEventObserver.h"
#include "shared/ServiceProvider.h"
#include "server/ServerConsole.h"
#include "shared/network/IServerCallback.h"
#include <map>

class SDLBackend: public NonCopyable, public IEventObserver, public IServerCallback {
private:
	bool _running;
	IFrontend *_frontend;
	EventHandler _eventHandler;
	typedef std::map<int, int16_t> KeyMap;
	typedef KeyMap::const_iterator KeyMapConstIter;
	KeyMap _keys;
	ServiceProvider _serviceProvider;

	void handleEvent (SDL_Event &event);
	void render ();
	void update (float deltaTime);
	void handleCommandLineArguments (int argc, char **argv, bool handleConfigVars);
	int init (int argc, char **argv);

	static void frameCallback (void *userdata);
	static int handleAppEvents (void *userdata, SDL_Event *event);

public:
	SDLBackend ();
	virtual ~SDLBackend ();

	bool isRunning () const;
	void mainLoop (int argc, char **argv);

	void runFrame ();

	// IEventObserver
	bool onKeyRelease (int32_t key) override;
	bool onKeyPress (int32_t key, int16_t modifier) override;

	// IServerCallback
	void onData (ClientId clientId, ByteStream &data) override;
	ProtocolMessagePtr onOOBData (const unsigned char *data) override;
	void onConnection (ClientId clientId) override;
	void onDisconnect (ClientId clientId) override;
};
