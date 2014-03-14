#pragma once

#include "engine/common/IFactoryRegistry.h"
#include "engine/IGame.h"
#include "engine/common/Singleton.h"
#include <string>

class GameFactoryContext {
public:
	virtual ~GameFactoryContext() {
	}
};

class IGameFactory: public IFactory<IGame, GameFactoryContext> {
public:
	virtual ~IGameFactory() {
	}
	virtual GamePtr create(const GameFactoryContext *ctx) const = 0;
};

class GameRegistry: public IFactoryRegistry<std::string, IGame, GameFactoryContext> {
private:
	typedef std::map<std::string, GamePtr> Games;
	typedef Games::iterator GamesIter;
	Games _games;

public:
	GameRegistry();
	virtual ~GameRegistry();

	GamePtr getGame (const std::string& id = "");
};

class GameRegisterStatic {
private:
	class StaticGameFactory: public IGameFactory {
	private:
		GamePtr _game;
	public:
		StaticGameFactory(GamePtr& game) :
				_game(game) {
		}

		GamePtr create(const GameFactoryContext *ctx) const override {
			return _game;
		}
	};

	StaticGameFactory _factory;
public:
	GameRegisterStatic(const std::string& id, GamePtr game) : _factory(game) {
		Singleton<GameRegistry>::getInstance().registerFactory(id, _factory);
	}
};
