#pragma once

#include "engine/common/NonCopyable.h"
#include "engine/common/IFrontend.h"
#include "engine/common/IEventObserver.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/ConsoleFrontend.h"
#include "engine/common/network/IClientCallback.h"

class BotBackend: public NonCopyable, public IEventObserver, IClientCallback {
private:
	bool _running;
	ServiceProvider _serviceProvider;
	TextConsole _console;
	ConsoleFrontend _frontend;
	EventHandler _eventHandler;

	// command callbacks
	void connect (const ICommand::Args& args);
	void ping ();

	// IClientCallback
	void onData (ByteStream &data) override;
	void onOOBData (const std::string& host, const IProtocolMessage* message) override;

	int init (int argc, char **argv);
	void runFrame ();
public:
	BotBackend ();
	virtual ~BotBackend ();

	void mainLoop (int argc, char **argv);
};
