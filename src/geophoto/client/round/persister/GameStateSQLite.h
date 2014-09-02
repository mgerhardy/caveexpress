#pragma once

#include "geophoto/client/round/IGameStatePersister.h"
#include "engine/common/SQLite.h"
#include <stdint.h>

class GameStateSQLite: public SQLite, public IGameStatePersister {
public:
	GameStateSQLite ();

	bool saveRound (const GameRound& round) override;
	bool loadRound (GameRound& round) override;
};
