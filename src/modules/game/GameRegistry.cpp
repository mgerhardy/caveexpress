#include "game/GameRegistry.h"
#include "common/System.h"

GameRegistry::GameRegistry()
{
}

GameRegistry::~GameRegistry()
{
	for (auto i = _games.begin(); i != _games.end(); ++i)
		delete i->second;
	_games.clear();
}

GamePtr GameRegistry::getGame (const std::string& id)
{
	GameFactoryContext ctx;
	GamePtr game;
	if (id.empty()) {
		if (!getFactories().empty()) {
			FactoryMapConstIter i = getFactories().begin();
			const IFactory<IGame, GameFactoryContext> *factory = i->second;
			game = factory->create(&ctx);
			if (!game)
				System.exit("failed to create game factory: " + i->first, 1);
		} else {
			System.exit("no game factory registered", 1);
		}
	} else {
		GamesIter i = _games.find(id);
		if (i != _games.end())
			return i->second;
		game = create(id, &ctx);
		if (!game) {
			System.exit("no game factory found for " + id, 1);
		}
	}

	_games.insert(std::make_pair(id, game));
	return game;
}
