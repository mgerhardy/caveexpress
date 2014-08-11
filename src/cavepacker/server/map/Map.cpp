#include "Map.h"
#include "engine/server/box2d/DebugRenderer.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/MapSettings.h"
#include "engine/common/Shared.h"
#include "engine/common/Logger.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/SpriteDefinition.h"
#include "engine/common/IFrontend.h"
#include "engine/common/Math.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/IMapContext.h"
#include "engine/common/network/messages/InitDoneMessage.h"
#include "engine/common/network/messages/SoundMessage.h"
#include "engine/common/network/messages/MapSettingsMessage.h"
#include "engine/common/network/messages/TextMessage.h"
#include "engine/common/network/messages/SpawnInfoMessage.h"
#include "engine/common/network/messages/LoadMapMessage.h"
#include "engine/common/network/messages/AddEntityMessage.h"
#include "engine/common/network/messages/RemoveEntityMessage.h"
#include "engine/common/network/messages/UpdateEntityMessage.h"
#include "engine/common/network/messages/MapRestartMessage.h"
#include "engine/common/network/messages/UpdatePointsMessage.h"
#include "engine/common/network/messages/PauseMessage.h"
#include "engine/common/CommandSystem.h"
#include "engine/common/FileSystem.h"
#include "engine/common/System.h"
#include "engine/common/vec2.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/Commands.h"
#include "cavepacker/server/map/SokubanMapContext.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"
#include <SDL.h>
#include <algorithm>
#include <functional>
#include <cassert>
#include <climits>

#define INDEX(col, row) ((col) + _width * (row))

Map::Map () :
		IMap(), _frontend(nullptr), _serviceProvider(nullptr), _forcedFinish(false), _autoSolve(false), _nextSolveStep(0)
{
	Commands.registerCommand(CMD_MAP_PAUSE, bind(Map, triggerPause));
	Commands.registerCommand(CMD_MAP_RESTART, bind(Map, triggerRestart));
	Commands.registerCommand(CMD_START, bind(Map, startMap));
	Commands.registerCommand(CMD_FINISHMAP, bind(Map, finishMap));
	Commands.registerCommand("map_print", bind(Map, printMap));
	Commands.registerCommand("solve", bind(Map, solveMap));
	Commands.registerCommand("undo", bind(Map, undo));

	resetCurrentMap();
}

Map::~Map ()
{
	Commands.removeCommand(CMD_MAP_PAUSE);
	Commands.removeCommand(CMD_MAP_RESTART);
	Commands.removeCommand(CMD_START);
	Commands.removeCommand(CMD_FINISHMAP);
	Commands.removeCommand("map_print");
	Commands.removeCommand("solve");
	Commands.removeCommand("undo");
}

void Map::shutdown ()
{
	resetCurrentMap();
}

int Map::solve ()
{
	triggerRestart();
	_autoSolve = true;
	_serviceProvider->getNetwork().sendToAllClients(AutoSolveStartedMessage());
	return _solution.size();
}

std::string Map::getSolution() const
{
	const FilePtr& filePtr = FS.getFile(FS.getMapsDir() + _name + ".sol");
	if (!filePtr) {
		info(LOG_SERVER, "no solution file found for " + _name);
		return "";
	}

	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	if (!buffer || fileLen <= 0) {
		error(LOG_SERVER, "solution file " + filePtr->getURI().print() + " can't get loaded");
		return "";
	}

	std::string solution = string::toLower(std::string(buffer, fileLen));
	for (std::string::iterator i = solution.begin(); i != solution.end(); ++i) {
		if (!isdigit(*i))
			continue;
		std::string digit;
		digit += *i;
		for (++i; isdigit(*i) && i != solution.end(); ++i) {
			digit += *i;
		}
		const int n = string::toInt(digit);
		if (i == solution.end()) {
			error(LOG_SERVER, "invalid rle encoded solution found");
			break;
		}
		if (*i != '(') {
			std::string repeat;
			repeat += *i;
			const std::string r = repeat;
			for (int k = 1; k < n; ++k) {
				repeat += r;
			}
			solution = string::replaceAll(solution, digit + r, repeat);
			i = solution.begin();
			continue;
		}
		std::string repeat;
		for (++i; i != solution.end(); ++i) {
			if (*i == ')') {
				++i;
				break;
			}
			repeat += *i;
		}
		const std::string r = repeat;
		for (int k = 1; k < n; ++k) {
			repeat += r;
		}
		solution = string::replaceAll(solution, digit + "(" + r + ")", repeat);
		i = solution.begin();
	}
	return solution;
}

void Map::sendSound (int clientMask, const SoundType& type, const b2Vec2& pos) const
{
	const SoundMessage msg(pos.x, pos.y, type);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::disconnect (ClientId clientId)
{
	removePlayer(clientId);

	_serviceProvider->getNetwork().disconnectClientFromServer(clientId);

	if (_players.size() == 1 && _playersWaitingForSpawn.empty())
		resetCurrentMap();
}

void Map::triggerRestart ()
{
	if (!_serviceProvider->getNetwork().isServer())
		return;

	info(LOG_MAP, "trigger restart");
	Commands.executeCommandLine(CMD_MAP_START " " + getName());
}

inline bool Map::isActive () const
{
	const bool noEntities = _entities.empty();
	if (noEntities)
		return false;
	const bool noPlayers = _players.empty();
	if (noPlayers)
		return false;
	return true;
}

void Map::finishMap ()
{
#ifdef DEBUG
	_forcedFinish = true;
#endif
}

Player* Map::getPlayer (ClientId clientId)
{
	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		if ((*i)->getClientId() == clientId) {
			return *i;
		}
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		if ((*i)->getClientId() == clientId) {
			return *i;
		}
	}

	error(LOG_MAP, String::format("no player found for the client id %i", clientId));
	return nullptr;
}

bool Map::isDone () const
{
	if (_forcedFinish)
		return true;
	if (isFailed())
		return false;
	for (StateMapConstIter i = _state.begin(); i != _state.end(); ++i) {
		// if there is an empty target left, we are not yet done
		if (i->second == Sokuban::TARGET || i->second == Sokuban::PLAYERONTARGET)
			return false;
	}
	return true;
}

void Map::increaseMoves ()
{
	++_moves;
	debug(LOG_SERVER, String::format("moved fields: %i", _moves));
	_serviceProvider->getNetwork().sendToAllClients(UpdatePointsMessage(_moves));
}

void Map::undo ()
{
	if (_autoSolve)
		return;

	if (_moves <= 0)
		return;

	if (_players.empty())
		return;
	Player *player = *_players.begin();
	player->undo();

	--_moves;
	debug(LOG_SERVER, String::format("moved fields after undo: %i", _moves));
	_serviceProvider->getNetwork().sendToAllClients(UpdatePointsMessage(_moves));
}

void Map::undoPackage (int col, int row, int targetCol, int targetRow)
{
	MapTile* package = getPackage(col, row);
	if (package != nullptr) {
		info(LOG_SERVER, "move package back");
		rebuildField();
		package->setPos(targetCol, targetRow);
		rebuildField();
		--_pushes;
	} else {
		info(LOG_SERVER, "dont move package back");
	}
}

void Map::abortAutoSolve ()
{
	_autoSolve = false;
	_serviceProvider->getNetwork().sendToAllClients(AutoSolveAbortedMessage());
}

bool Map::movePlayer (Player* player, char step)
{
	int x;
	int y;
	getXY(step, x, y);
	debug(LOG_SERVER, String::format("move player %i:%i (current: %i:%i)", x, y, player->getCol(), player->getRow()));
	// move player and move touching packages
	const int targetCol = player->getCol() + x;
	const int targetRow = player->getRow() + y;
	MapTile* package = getPackage(targetCol, targetRow);
	if (package != nullptr) {
		const int pCol = targetCol + x;
		const int pRow = targetRow + y;
		if (!isFree(pCol, pRow)) {
			debug(LOG_SERVER, "can't move here - can't move package. target field is blocked");
			return false;
		}
		if (!package->setPos(pCol, pRow)) {
			debug(LOG_SERVER, "failed to move the package - thus can't move the player");
			return false;
		}
		debug(LOG_SERVER, "moved package");
		increasePushes();
		rebuildField();
		// sokoban standard - if a package was moved, the move char is uppercase
		step = toupper(step);
	}
	if (!player->setPos(targetCol, targetRow)) {
		debug(LOG_SERVER, "failed to move the player");
		return false;
	}

	player->storeStep(step);
	increaseMoves();
	return true;
}

void Map::increasePushes ()
{
	++_pushes;
}

bool Map::isFailed () const
{
	if (_players.empty())
		return true;

	const uint16_t limit = USHRT_MAX - 10;
	if (_moves >= limit || _pushes >= limit)
		return true;

	return false;
}

void Map::restart (uint32_t delay)
{
	if (_restartDue > 0)
		return;

	info(LOG_MAP, "trigger map restart");
	_restartDue = SDL_GetTicks() + delay;
	const MapRestartMessage msg(delay);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void Map::resetCurrentMap ()
{
	_autoSolve = false;
	_nextSolveStep = 0;
	_solution = "";
	_timeManager.reset();
	if (!_name.empty()) {
		const CloseMapMessage msg;
		_serviceProvider->getNetwork().sendToAllClients(msg);
		info(LOG_MAP, "reset map: " + _name);
	}
	_field.clear();
	_state.clear();
	_forcedFinish = false;
	_moves = 0;
	_restartDue = 0;
	_pause = false;
	_mapRunning = false;
	_width = 0;
	_height = 0;
	_time = 0;
	_entityRemovalAllowed = true;
	if (!_name.empty())
		info(LOG_MAP, "* clear map");

	{ // now free the allocated memory
		for (EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
			delete *i;
		}
		for (EntityListIter i = _entitiesToAdd.begin(); i != _entitiesToAdd.end(); ++i) {
			delete *i;
		}
		_entitiesToAdd.clear();
		_entities.clear();
		_entities.reserve(400);

		for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
			delete *i;
		}
		_players.clear();
		_players.reserve(MAX_CLIENTS);
		if (!_name.empty())
			info(LOG_MAP, "* removed allocated memory");
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		delete *i;
	}
	_playersWaitingForSpawn.clear();
	_playersWaitingForSpawn.reserve(MAX_CLIENTS);

	if (!_name.empty())
		info(LOG_MAP, "done with resetting: " + _name);
	_name.clear();
}

inline IMapContext* getMapContext (const std::string& name)
{
	return new SokubanMapContext(name);
}

inline const EntityType& getEntityTypeForSpriteType (const SpriteType& spriteType)
{
	if (SpriteTypes::isTarget(spriteType))
		return EntityTypes::TARGET;
	if (SpriteTypes::isGround(spriteType))
		return EntityTypes::GROUND;
	if (SpriteTypes::isSolid(spriteType))
		return EntityTypes::SOLID;
	if (SpriteTypes::isPackage(spriteType))
		return EntityTypes::PACKAGE;
	System.exit("unknown sprite type given: " + spriteType.name, 1);
	return EntityType::NONE;
}

bool Map::load (const std::string& name)
{
	ScopedPtr<IMapContext> ctx(getMapContext(name));

	resetCurrentMap();

	if (name.empty()) {
		info(LOG_MAP, "no map name given");
		return false;
	}

	info(LOG_MAP, "load map " + name);

	if (!ctx->load(false)) {
		error(LOG_MAP, "failed to load the map " + name);
		return false;
	}

	ctx->save();
	_settings = ctx->getSettings();
	_name = ctx->getName();
	_title = ctx->getTitle();
	_width = getSetting(msn::WIDTH, "-1").toInt();
	_height = getSetting(msn::HEIGHT, "-1").toInt();
	_solution = getSolution();
	const std::string solutionSteps = string::toString(_solution.length());
	_settings.insert(std::make_pair("best", solutionSteps));
	info(LOG_MAP, "Solution has " + solutionSteps + " steps");

	if (_width <= 0 || _height <= 0) {
		error(LOG_MAP, "invalid map dimensions given");
		return false;
	}

	const std::vector<MapTileDefinition>& mapTileList = ctx->getMapTileDefinitions();
	for (std::vector<MapTileDefinition>::const_iterator i = mapTileList.begin(); i != mapTileList.end(); ++i) {
		const SpriteType& t = i->spriteDef->type;
		info(LOG_MAP, "sprite type: " + t.name + ", " + i->spriteDef->id);
		MapTile *mapTile = new MapTile(*this, i->x, i->y, getEntityTypeForSpriteType(t));
		mapTile->setSpriteID(i->spriteDef->id);
		mapTile->setAngle(randBetweenf(-0.1, 0.1f));
		loadEntity(mapTile);
	}

	info(LOG_MAP, String::format("map loading done with %i tiles", mapTileList.size()));

	ctx->onMapLoaded();

	_frontend->onMapLoaded();
	const LoadMapMessage msg(_name, _title);
	_serviceProvider->getNetwork().sendToAllClients(msg);

	_mapRunning = true;
	return true;
}

bool Map::spawnPlayer (Player* player)
{
	assert(_entityRemovalAllowed);

	const int col = getSetting(msn::PLAYER_X).toInt();
	const int row = getSetting(msn::PLAYER_Y).toInt();
	if (!player->setPos(col, row))
		error(LOG_SERVER, String::format("failed to set the player position to %i:%i", col, row));
	player->onSpawn();
	addEntity(0, *player);
	info(LOG_SERVER, "spawned player " + player->toString());
	_players.push_back(player);
	return true;
}

void Map::sendMessage (ClientId clientId, const std::string& message) const
{
	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(TextMessage(message));
}

bool Map::isReadyToStart () const
{
	return _playersWaitingForSpawn.size() > 1;
}

std::string Map::getMapString() const {
	std::stringstream ss;
	for (int row = 0; row < _height; ++row) {
		for (int col = 0; col < _width; ++col) {
			const StateMapConstIter& i = _state.find(INDEX(col, row));
			if (i == _state.end()) {
				ss << '+';
				continue;
			}
			const char c = i->second;
			ss << c;
		}
		ss << '\n';
	}
	return ss.str();
}

void Map::printMap ()
{
	const std::string& mapString = getMapString();
	info(LOG_CLIENT, "\n" + mapString);
}

void Map::startMap ()
{
	rebuildField();
	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		spawnPlayer(*i);
	}
	_playersWaitingForSpawn.clear();

	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(StartMapMessage());
}

MapTile* Map::getPackage (int col, int row)
{
	FieldMapIter i = _field.find(INDEX(col, row));
	if (i == _field.end()) {
		return nullptr;
	}
	if (EntityTypes::isPackage(i->second->getType())) {
		return static_cast<MapTile*>(i->second);
	}
	return nullptr;
}

bool Map::isFree (int col, int row)
{
	StateMapConstIter i = _state.find(INDEX(col, row));
	// not part of the map - thus, not free
	if (i == _state.end()) {
		debug(LOG_MAP, String::format("col: %i, row: %i is not part of the map", col, row));
		return false;
	}

	const char c = i->second;
	debug(LOG_MAP, String::format("col: %i, row: %i is of type '%c'", col, row, c));
	return c == Sokuban::GROUND || c == Sokuban::TARGET;
}

bool Map::isTarget (int col, int row)
{
	StateMapConstIter i = _state.find(INDEX(col, row));
	if (i == _state.end())
		return false;
	const char c = i->second;
	return c == Sokuban::PACKAGEONTARGET || c == Sokuban::PLAYERONTARGET || c == Sokuban::TARGET;
}

bool Map::isPackage (int col, int row)
{
	StateMapConstIter i = _state.find(INDEX(col, row));
	if (i == _state.end())
		return false;
	const char c = i->second;
	return c == Sokuban::PACKAGE || c == Sokuban::PACKAGEONTARGET;
}

bool Map::initPlayer (Player* player)
{
	if (!_mapRunning)
		return false;

	if (getPlayer(player->getClientId()) != nullptr)
		return false;

	assert(_entityRemovalAllowed);

	INetwork& network = _serviceProvider->getNetwork();
	const ClientId clientId = player->getClientId();
	info(LOG_SERVER, "init player " + player->toString());
	const MapSettingsMessage mapSettingsMsg(_settings);
	network.sendToClient(clientId, mapSettingsMsg);

	const InitDoneMessage msgInit(player->getID(), 0, 0, 0);
	network.sendToClient(clientId, msgInit);

	network.sendToClient(clientId, InitWaitingMapMessage());
	sendMapToClient(clientId);
	if (!_players.empty()) {
		const bool spawned = spawnPlayer(player);
		return spawned;
	}
	_playersWaitingForSpawn.push_back(player);
	return true;
}

void Map::printPlayersList () const
{
	for (PlayerListConstIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		const std::string& name = (*i)->getName();
		info(LOG_SERVER, "* " + name + " (waiting)");
	}
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const std::string& name = (*i)->getName();
		info(LOG_SERVER, "* " + name + " (spawned)");
	}
}

void Map::sendPlayersList () const
{
	std::vector<std::string> names;
	for (PlayerListConstIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		const std::string& name = (*i)->getName();
		names.push_back(name);
	}
	INetwork& network = _serviceProvider->getNetwork();
	network.sendToAllClients(PlayerListMessage(names));
}

void Map::removeEntity (int clientMask, const IEntity& entity) const
{
	const RemoveEntityMessage msg(entity.getID(), false);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

char Map::getSokubanFieldId (const IEntity *entity) const
{
	const EntityType& t = entity->getType();
	if (EntityTypes::isGround(t))
		return Sokuban::GROUND;
	if (EntityTypes::isTarget(t))
		return Sokuban::TARGET;
	// the order matters
	if (EntityTypes::isSolid(t))
		return Sokuban::WALL;
	if (EntityTypes::isPackage(t))
		return Sokuban::PACKAGE;
	if (EntityTypes::isPlayer(t))
		return Sokuban::PLAYER;
	return Sokuban::GROUND;
}

bool Map::setField (IEntity *entity, int col, int row)
{
	const int index = INDEX(col, row);
	FieldMapConstIter fi = _field.find(index);
	if (fi == _field.end()) {
		_field[index] = entity;
	} else {
		// ground and target have low prio
		if (fi->second->isGround() || fi->second->isTarget())
			_field[index] = entity;
	}
	StateMapConstIter i = _state.find(index);
	if (i == _state.end()) {
		_state[index] = getSokubanFieldId(entity);
		return true;
	}
	const char c = i->second;
	char nc = getSokubanFieldId(entity);
	if (c == Sokuban::PLAYER) {
		if (nc == Sokuban::TARGET)
			nc = Sokuban::PLAYERONTARGET;
		else if (nc == Sokuban::GROUND)
			nc = c;
		else
			return false;
	} else if (c == Sokuban::PACKAGE) {
		if (nc == Sokuban::TARGET)
			nc = Sokuban::PACKAGEONTARGET;
		else if (nc == Sokuban::GROUND)
			nc = c;
		else
			return false;
	} else if (c == Sokuban::TARGET) {
		if (nc == Sokuban::PACKAGE)
			nc = Sokuban::PACKAGEONTARGET;
		else if (nc == Sokuban::PLAYER)
			nc = Sokuban::PLAYERONTARGET;
		else if (nc == Sokuban::GROUND)
			nc = c;
		else
			return false;
	}
	_state[index] = nc;
	return true;
}

void Map::addEntity (IEntity *entity)
{
	entity->onSpawn();
	_entitiesToAdd.push_back(entity);
}

void Map::addEntity (int clientMask, const IEntity& entity) const
{
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const AddEntityMessage msg(entity.getID(), entity.getType(), Animation::NONE,
			entity.getSpriteID(), entity.getCol(), entity.getRow(), 1.0f, 1.0f, angle, ENTITY_ALIGN_LOWER_LEFT);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::updateEntity (int clientMask, const IEntity& entity) const
{
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const UpdateEntityMessage msg(entity.getID(), entity.getCol(), entity.getRow(), angle, 0);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::sendMapToClient (ClientId clientId) const
{
	const int clientMask = ClientIdToClientMask(clientId);
	for (EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		const IEntity* e = *i;
		if (!e->isMapTile() && !e->isPackage())
			continue;
		addEntity(clientMask, *e);
	}
}

void Map::loadEntity (IEntity *entity)
{
	assert(_entityRemovalAllowed);
	_entities.push_back(entity);
}

bool Map::removePlayer (ClientId clientId)
{
	assert(_entityRemovalAllowed);

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		if ((*i)->getClientId() != clientId)
			continue;
		(*i)->remove();
		delete *i;
		_playersWaitingForSpawn.erase(i);
		sendPlayersList();
		return true;
	}

	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		if ((*i)->getClientId() != clientId)
			continue;

		removeEntity(0, **i);
		(*i)->remove();
		delete *i;
		_players.erase(i);
		return true;
	}
	error(LOG_MAP, String::format("could not find the player with the clientId %i", clientId));
	return false;
}

bool Map::visitEntity (IEntity *entity)
{
	entity->update(Constant::DELTA_PHYSICS_MILLIS);
	return false;
}

void Map::rebuildField ()
{
	_field.clear();
	_state.clear();
	for (EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
		if (!setField(*i, (*i)->getCol(), (*i)->getRow()))
			System.exit("invalid map state", 1);
	}
	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		if (!setField(*i, (*i)->getCol(), (*i)->getRow()))
			System.exit("invalid map state", 1);
	}
}

void Map::update (uint32_t deltaTime)
{
	if (_pause)
		return;

	_timeManager.update(deltaTime);

	_time += deltaTime;

	ExecutionTime visitTime("VisitTime", 2000L);
	visitEntities(this);

	rebuildField();

	if (_restartDue > 0 && _restartDue <= SDL_GetTicks()) {
		const std::string currentName = getName();
		info(LOG_MAP, "restarting map " + currentName);
		load(currentName);
	}
	handleAutoSolve(deltaTime);
}

void Map::handleAutoSolve (uint32_t deltaTime)
{
	if (!_autoSolve)
		return;

	if (_solution.empty()) {
		_autoSolve = false;
		error(LOG_SERVER, "no solution");
		return;
	}

	_nextSolveStep -= deltaTime;
	if (_nextSolveStep > 0)
		return;

	_nextSolveStep = Config.getConfigVar("solvestepmillis", "100")->getIntValue();

	if (_solution[0] == '(') {
		_autoSolve = false;
		error(LOG_SERVER, "x() repeat syntax is not supported");
		return;
	}

	int l = 0;
	for (std::string::iterator i = _solution.begin(); i != _solution.end(); ++i) {
		if (*i >= '0' && *i <= '9') {
			++l;
		} else {
			break;
		}
	}

	if (_players.empty()) {
		_autoSolve = false;
		error(LOG_SERVER, "no player connected");
		return;
	}
	Player *p = *_players.begin();
	if (l == 0) {
		const char step = _solution[0];
		movePlayer(p, step);
		_solution = _solution.substr(1);
		return;
	}

	const std::string& rle = _solution.substr(0, l);
	const int n = string::toInt(rle);
	if (n <= 1) {
		_solution = _solution.substr(l);
		movePlayer(p, _solution[0]);
		_solution = _solution.substr(1);
	} else {
		movePlayer(p, _solution[l]);
		_solution = string::toString(n - 1) + _solution.substr(l);
	}
}

const IEntity* Map::getEntity (int16_t id) const
{
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i)
		if ((*i)->getID() == id)
			return *i;
	for (Map::EntityListConstIter i = _entities.begin(); i != _entities.end(); ++i)
		if ((*i)->getID() == id)
			return *i;

	return nullptr;
}

void Map::visitEntities (IEntityVisitor *visitor, const EntityType& type)
{
	if (type == EntityType::NONE || type == EntityTypes::PLAYER) {
		bool needUpdate = false;
		for (PlayerListIter i = _players.begin(); i != _players.end();) {
			Player* e = *i;
			if (visitor->visitEntity(e)) {
				debug(LOG_SERVER, String::format("remove player by visit %i: %s", e->getID(), e->getType().name.c_str()));
				removeEntity(ClientIdToClientMask(e->getClientId()), *e);
				delete *i;
				i = _players.erase(i);
				needUpdate = true;
			} else {
				++i;
			}
		}
		if (needUpdate) {
			if (_players.empty()) {
				resetCurrentMap();
				return;
			}
		}
	}

	// changing the entities list is not allowed here. Adding or removing
	// would invalidate the iterators
	for (Map::EntityListIter i = _entities.begin(); i != _entities.end();) {
		IEntity* e = *i;
		if (type.isNone() || e->getType() == type) {
			if (visitor->visitEntity(e)) {
				debug(LOG_SERVER, String::format("remove entity by visit %i: %s", e->getID(), e->getType().name.c_str()));
				removeEntity(0, *e);
				(*i)->remove();
				delete *i;
				i = _entities.erase(i);
			} else {
				++i;
			}
		} else {
			++i;
		}
	}

	// now we will add the newly added entities to the list to not invalidate the iterators
	for (Map::EntityListIter i = _entitiesToAdd.begin(); i != _entitiesToAdd.end(); ++i) {
		_entities.push_back(*i);
	}
	_entitiesToAdd.clear();
}

void Map::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;
}

void Map::triggerPause ()
{
	if (!_serviceProvider->getNetwork().isServer())
		return;
	_pause ^= true;
	const PauseMessage msg(_pause);
	_serviceProvider->getNetwork().sendToAllClients(msg);
	info(LOG_MAP, String::format("pause: %s", _pause ? "true" : "false"));
}
