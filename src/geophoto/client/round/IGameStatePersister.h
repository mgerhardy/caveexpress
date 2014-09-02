#pragma once

#include "engine/common/Compiler.h"

class GameRound;

class IGameStatePersister {
public:
	IGameStatePersister ()
	{
	}

	virtual ~IGameStatePersister ()
	{
	}

	virtual bool saveRound (const GameRound& round) = 0;
	virtual bool loadRound (GameRound& round) = 0;
};

class NOPPersister: public IGameStatePersister {
public:
	NOPPersister () :
			IGameStatePersister()
	{
	}

	virtual bool saveRound (const GameRound& round) override
	{
		return true;
	}

	virtual bool loadRound (GameRound& round) override
	{
		return true;
	}
};
