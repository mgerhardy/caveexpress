#pragma once

#include "client/round/IGameStatePersister.h"
#include "shared/SQLite.h"
#include <stdint.h>

class GameStateSQLite: public SQLite, public IGameStatePersister {
public:
	GameStateSQLite ();

	bool saveRound (const GameRound& round) override;
	bool loadRound (GameRound& round) override;
};
