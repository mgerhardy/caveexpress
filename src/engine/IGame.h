#pragma once

#include "engine/client/ui/UI.h"
#include "engine/common/Pointers.h"
#include "engine/common/network/IProtocolHandler.h"
#include "engine/common/MapManager.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Commands.h"
#include "engine/common/CommandSystem.h"

// TODO: rename methods and document stuff
class IGame {
public:
	virtual ~IGame() {
	}

	virtual void initSoundCache () {}

	// called to connect to a local running server
	virtual void connect () {
		const std::string command = CMD_CL_CONNECT " localhost " + string::toString(Config.getPort());
		Commands.executeCommandLine(command);
	}

	// create the windows
	virtual void initUI (IFrontend* frontend, ServiceProvider& serviceProvider) {}

	virtual void update (uint32_t deltaTime) {}

	virtual std::string getMapName () { return ""; }

	virtual void shutdown () {}

	virtual int getPlayers () { return -1; }

	virtual void connect (ClientId clientId) {}

	virtual int disconnect (ClientId clientId) { return -1; }

	virtual void init (IFrontend *frontend, ServiceProvider& serviceProvider) {}

	virtual void mapReload () {}

	virtual bool mapLoad (const std::string& map) { return false; }

	virtual void mapShutdown () {}

	virtual IMapManager* getMapManager () { return nullptr; }
};

typedef SharedPtr<IGame> GamePtr;
