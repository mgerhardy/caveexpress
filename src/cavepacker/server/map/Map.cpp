#include "Map.h"
#include "common/ConfigManager.h"
#include "common/MapSettings.h"
#include "common/Shared.h"
#include "common/Log.h"
#include "service/ServiceProvider.h"
#include "common/SpriteDefinition.h"
#include "common/IFrontend.h"
#include "common/Math.h"
#include "network/INetwork.h"
#include "common/IMapContext.h"
#include "network/messages/InitDoneMessage.h"
#include "network/messages/SoundMessage.h"
#include "network/messages/MapSettingsMessage.h"
#include "network/messages/TextMessage.h"
#include "network/messages/SpawnInfoMessage.h"
#include "network/messages/LoadMapMessage.h"
#include "network/messages/AddEntityMessage.h"
#include "network/messages/RemoveEntityMessage.h"
#include "network/messages/UpdateEntityMessage.h"
#include "network/messages/MapRestartMessage.h"
#include "network/messages/UpdatePointsMessage.h"
#include "network/messages/PauseMessage.h"
#include "common/CommandSystem.h"
#include "common/FileSystem.h"
#include "common/System.h"
#include "common/vec2.h"
#include "common/ExecutionTime.h"
#include "common/Commands.h"
#include "cavepacker/server/map/SokobanMapContext.h"
#include "cavepacker/server/map/Pathfinding.h"
#include "cavepacker/shared/CavePackerSpriteType.h"
#include "cavepacker/shared/EntityStates.h"
#include "cavepacker/shared/network/messages/ShowDeadlocksMessage.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"
#include <SDL.h>
#include <algorithm>
#include <functional>
#include <cassert>
#include <climits>

namespace cavepacker {

Map::Map () :
		IMap(), _frontend(nullptr), _serviceProvider(nullptr), _forcedFinish(false), _autoSolve(false), _nextSolveStep(0)
{
	Commands.registerCommand(CMD_MAP_PAUSE, bindFunction(Map, triggerPause));
	Commands.registerCommand(CMD_MAP_RESTART, bindFunction(Map, triggerRestart));
	Commands.registerCommand(CMD_START, bindFunction(Map, startMap));
	Commands.registerCommand(CMD_FINISHMAP, bindFunction(Map, finishMap));
	Commands.registerCommand("map_print", bindFunction(Map, printMap));
	Commands.registerCommand("solve", bindFunction(Map, solveMap));

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
	const FilePtr& filePtr = FS.getFileFromURL("maps://" + _name + ".sol");
	if (!filePtr) {
		Log::info(LOG_SERVER, "no solution file found for %s", _name.c_str());
		return "";
	}

	char *buffer;
	const int fileLen = filePtr->read((void **) &buffer);
	std::unique_ptr<char[]> p(buffer);
	if (!buffer || fileLen <= 0) {
		Log::error(LOG_SERVER, "solution file '%s' can't get loaded", filePtr->getName().c_str());
		return "";
	}

	std::string solution = string::toLower(std::string(buffer, fileLen));
	for (std::string::iterator i = solution.begin(); i != solution.end(); ++i) {
		if (!isdigit(*i))
			continue;
		std::string digit;
		digit += *i;
		for (++i; i != solution.end(); ++i) {
			if (!isdigit(*i))
				break;
			digit += *i;
		}
		const int n = string::toInt(digit);
		if (i == solution.end()) {
			Log::error(LOG_SERVER, "invalid rle encoded solution found");
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

	Log::info(LOG_MAP, "trigger restart");
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

	Log::error(LOG_MAP, "no player found for the client id %i", clientId);
	return nullptr;
}

bool Map::isDone () const
{
	if (_forcedFinish)
		return true;
	if (isFailed())
		return false;
	return _state.isDone();
}

void Map::increaseMoves ()
{
	++_moves;
	Log::debug(LOG_SERVER, "moved fields: %i", _moves);
	_serviceProvider->getNetwork().sendToAllClients(UpdatePointsMessage(_moves));
}

void Map::undo (Player* player)
{
	if (_autoSolve)
		return;

	if (_moves <= 0)
		return;

	if (!player->undo())
		return;

	--_moves;
	Log::debug(LOG_SERVER, "moved fields after undo: %i", _moves);
	_serviceProvider->getNetwork().sendToAllClients(UpdatePointsMessage(_moves));
}

void Map::moveTo (Player* player, int col, int row)
{
	player->setTargetIndex(_state.getIndex(col, row));
	Log::info(LOG_SERVER, "move player %i from %i:%i to %i:%i", player->getID(), player->getCol(), player->getRow(), col, row);
}

bool Map::isAt (IEntity* entity, int index) const
{
	int col;
	int row;
	if (!_state.getColRowFromIndex(index, col, row))
		return false;
	return entity->getCol() == col && entity->getRow() == row;
}

int Map::getPositionIndex (IEntity* entity) const
{
	return _state.getIndex(entity->getCol(), entity->getRow());
}

char Map::getDirectionForMove (int currentIndex, int targetIndex) const
{
	std::vector<int> path;
	if (!astar(_state, currentIndex, targetIndex, path)) {
		Log::warn(LOG_MAP, "Could not find path from %i to %i", currentIndex, targetIndex);
		return '\0';
	}
	const int index = path[1];
	Log::debug(LOG_MAP, "walk to index %i (current: %i)", index, currentIndex);
	if (index - 1 == currentIndex) {
		return MOVE_RIGHT;
	} else if (index + 1 == currentIndex) {
		return MOVE_LEFT;
	} else if (index > currentIndex) {
		return MOVE_DOWN;
	}

	return MOVE_UP;
}

bool Map::undoPackage (int col, int row, int targetCol, int targetRow)
{
	MapTile* package = getPackage(col, row);
	if (package == nullptr) {
		Log::info(LOG_SERVER, "don't move package back");
		return false;
	}
	Log::info(LOG_SERVER, "move package back");
	rebuildField();
	const int origCol = package->getCol();
	const int origRow = package->getRow();
	if (!package->setPos(targetCol, targetRow))
		return false;
	rebuildField();
	if (_state.isInvalid(targetCol, targetRow)) {
		package->setPos(origCol, origRow);
		return false;
	}

	const char c = _state.getField(targetCol, targetRow);
	if (isPackageOnTarget(c))
		package->setState(CavePackerEntityStates::DELIVERED);
	else
		package->setState(CavePackerEntityStates::NONE);

	auto i = std::find(_deadLocks.begin(), _deadLocks.end(), _state.getIndex(col, row));
	if (i != _deadLocks.end()) {
		_deadLocks.erase(i);
	}

	--_pushes;

	checkDeadlock();

	return true;
}

void Map::abortAutoSolve ()
{
	if (!_autoSolve)
		return;
	_autoSolve = false;
	_serviceProvider->getNetwork().sendToAllClients(AutoSolveAbortedMessage());
}

bool Map::movePlayer (Player* player, char step)
{
	int x;
	int y;
	getXY(step, x, y);
	Log::debug(LOG_SERVER, "move player %i:%i (current: %i:%i)", x, y, player->getCol(), player->getRow());
	// move player and move touching packages
	const int targetCol = player->getCol() + x;
	const int targetRow = player->getRow() + y;
	MapTile* package = getPackage(targetCol, targetRow);
	if (package != nullptr) {
		const int pCol = targetCol + x;
		const int pRow = targetRow + y;
		if (!_state.isFree(pCol, pRow)) {
			Log::debug(LOG_SERVER, "can't move here - can't move package. target field is blocked");
			return false;
		}
		if (!package->setPos(pCol, pRow)) {
			Log::debug(LOG_SERVER, "failed to move the package - thus can't move the player");
			return false;
		}
		Log::debug(LOG_SERVER, "moved package %i", package->getID());
		increasePushes();
		rebuildField();
		if (_state.isTarget(pCol, pRow)) {
			package->setState(CavePackerEntityStates::DELIVERED);
			Log::debug(LOG_SERVER, "mark package as delivered %i", package->getID());
		} else if (package->getState() == CavePackerEntityStates::DELIVERED) {
			Log::debug(LOG_SERVER, "reset package state %i", package->getID());
			package->setState(CavePackerEntityStates::NONE);
		}
		// sokoban standard - if a package was moved, the move char is uppercase
		step = toupper(step);
	}
	if (!player->setPos(targetCol, targetRow)) {
		Log::debug(LOG_SERVER, "failed to move the player");
		return false;
	}

	player->storeStep(step);
	increaseMoves();
	// if we moved a package, check the state
	if (package != nullptr)
		checkDeadlock();
	return true;
}

void Map::checkDeadlock () {
	if (!_state.hasDeadlock()) {
		return;
	}
	for (auto index : _deadLocks) {
		auto entity = _field[index];
		if (entity == nullptr) {
			continue;
		}
		if (EntityTypes::isPackage(entity->getType())) {
			MapTile* pkg = static_cast<MapTile*>(entity);
			if (pkg->getState() != CavePackerEntityStates::DEADLOCK) {
				pkg->setState(CavePackerEntityStates::NONE);
			}
		}
	}

	auto deadlocks = _state.getDeadlockDetector().getDeadlocks();
	bool newDeadlock = false;
	for (auto index : deadlocks) {
		auto entity = _field[index];
		if (entity == nullptr) {
			continue;
		}
		if (EntityTypes::isPackage(entity->getType())) {
			MapTile* pkg = static_cast<MapTile*>(entity);
			if (pkg->getState() != CavePackerEntityStates::DELIVERED) {
				pkg->setState(CavePackerEntityStates::DEADLOCK);
			}
		}
		auto i = std::find(_deadLocks.begin(), _deadLocks.end(), index);
		if (i == _deadLocks.end()) {
			_deadLocks.push_back(index);
			newDeadlock = true;
		}
	}
	if (newDeadlock) {
		static const TextMessage msg("Deadlock detected");
		_serviceProvider->getNetwork().sendToAllClients(msg);
	}
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

	Log::info(LOG_MAP, "trigger map restart");
	_restartDue = _time + delay;
	const MapRestartMessage msg(delay);
	_serviceProvider->getNetwork().sendToAllClients(msg);
}

void Map::resetCurrentMap ()
{
	abortAutoSolve();
	_nextSolveStep = 0;
	_solution = "";
	_deadLocks.clear();
	_timeManager.reset();
	if (!_name.empty()) {
		const CloseMapMessage msg;
		_serviceProvider->getNetwork().sendToAllClients(msg);
		Log::info(LOG_MAP, "reset map: %s", _name.c_str());
	}
	_field.clear();
	_state.clear();
	_forcedFinish = false;
	_moves = 0;
	_pushes = 0;
	_restartDue = 0;
	_pause = false;
	_mapRunning = false;
	_width = 0;
	_height = 0;
	_time = 0;
	_entityRemovalAllowed = true;
	if (!_name.empty())
		Log::info(LOG_MAP, "* clear map");

	{ // now free the allocated memory
		for (EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
			delete *i;
		}
		_entities.clear();
		_entities.reserve(400);

		for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
			delete *i;
		}
		_players.clear();
		_players.reserve(MAX_CLIENTS);
		if (!_name.empty())
			Log::info(LOG_MAP, "* removed allocated memory");
	}

	for (PlayerListIter i = _playersWaitingForSpawn.begin(); i != _playersWaitingForSpawn.end(); ++i) {
		delete *i;
	}
	_playersWaitingForSpawn.clear();
	_playersWaitingForSpawn.reserve(MAX_CLIENTS);

	if (!_name.empty())
		Log::info(LOG_MAP, "done with resetting: %s", _name.c_str());
	_name.clear();
}

inline IMapContext* getMapContext (const std::string& name)
{
	return new SokobanMapContext(name);
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
	std::unique_ptr<IMapContext> ctx(getMapContext(name));

	resetCurrentMap();

	if (name.empty()) {
		Log::info(LOG_MAP, "no map name given");
		return false;
	}

	Log::info(LOG_MAP, "load map %s", name.c_str());

	if (!ctx->load(false)) {
		Log::error(LOG_MAP, "failed to load the map %s", name.c_str());
		return false;
	}

	ctx->save();
	_settings = ctx->getSettings();
	_startPositions = ctx->getStartPositions();
	_name = ctx->getName();
	_title = ctx->getTitle();
	_width = getSetting(msn::WIDTH, "-1").toInt();
	_height = getSetting(msn::HEIGHT, "-1").toInt();
	_field.assign(_width * _height, nullptr);
	_state.setSize(_width, _height);
	_solution = getSolution();
	const std::string solutionSteps = string::toString(_solution.length());
	_settings.insert(std::make_pair("best", solutionSteps));
	Log::info(LOG_MAP, "Solution has %s steps", solutionSteps.c_str());

	if (_width <= 0 || _height <= 0) {
		Log::error(LOG_MAP, "invalid map dimensions given");
		return false;
	}

	const std::vector<MapTileDefinition>& mapTileList = ctx->getMapTileDefinitions();
	for (std::vector<MapTileDefinition>::const_iterator i = mapTileList.begin(); i != mapTileList.end(); ++i) {
		const SpriteType& t = i->spriteDef->type;
		Log::info(LOG_MAP, "sprite type: %s, %s", t.name.c_str(), i->spriteDef->id.c_str());
		MapTile *mapTile = new MapTile(*this, i->x, i->y, getEntityTypeForSpriteType(t));
		mapTile->setSpriteID(i->spriteDef->id);
		mapTile->setAngle(randBetweenf(-0.1, 0.1f));
		loadEntity(mapTile);
	}

	Log::info(LOG_MAP, "map loading done with %i tiles", (int)mapTileList.size());

	ctx->onMapLoaded();
	rebuildField();
	_state.initDeadlock();

	_frontend->onMapLoaded();
	const LoadMapMessage msg(_name, _title);
	_serviceProvider->getNetwork().sendToAllClients(msg);

	_mapRunning = true;
	return true;
}

bool Map::spawnPlayer (Player* player)
{
	assert(_entityRemovalAllowed);

	const int startPosIdx = _players.size();
	int col, row;
	if (!getStartPosition(startPosIdx, col, row)) {
		Log::error(LOG_SERVER, "no player position for index %i", startPosIdx);
		return false;
	}
	if (!player->setPos(col, row)) {
		Log::error(LOG_SERVER, "failed to set the player position to %i:%i", col, row);
		return false;
	}
	player->onSpawn();
	addEntity(0, *player);
	Log::info(LOG_SERVER, "spawned player %i", player->getID());
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

std::string Map::getMapString() const
{
	std::string mapStr;
	mapStr.reserve(_height * _width);
	for (int row = 0; row < _height; ++row) {
		for (int col = 0; col < _width; ++col) {
			if (_state.isInvalid(col, row)) {
				mapStr.append(" ");
				continue;
			}
			MapTile* package = getPackage(col, row);
			bool player = false;
			for (Player* p : getPlayers()) {
				if (p->getCol() == col && p->getRow() == row) {
					const int pc = _state.getField(col, row);
					if (pc == Sokoban::TARGET) {
						const char str[2] = { Sokoban::PLAYERONTARGET, '\0' };
						mapStr.append(str);
					} else {
						const char str[2] = { Sokoban::PLAYER, '\0' };
						mapStr.append(str);
					}
					player = true;
					break;
				}
			}
			if (player)
				continue;
			const char c = package != nullptr ? Sokoban::PACKAGE : _state.getField(col, row);
			const char str[2] = { c, '\0' };
			mapStr.append(str);
		}
		mapStr.append("\n");
	}
	return mapStr;
}

void Map::printMap ()
{
	const std::string& mapString = getMapString();
	Log::info(LOG_CLIENT, "Map State:\n%s", mapString.c_str());
	Log::info(LOG_CLIENT, "Board State:\n%s", _state.toString().c_str());
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

	for (int row = 0; row < _height; ++row) {
		for (int col = 0; col < _width; ++col) {
			if (_state.isInvalid(col, row)) {
				continue;
			}
			const char c = _state.getField(col, row);
			if (!isPackageOnTarget(c)) {
				continue;
			}
			MapTile* package = getPackage(col, row);
			package->setState(CavePackerEntityStates::DELIVERED);
		}
	}
}

MapTile* Map::getPackage (int col, int row) const
{
	auto i = _field[_state.getIndex(col, row)];
	if (i == nullptr) {
		return nullptr;
	}
	if (EntityTypes::isPackage(i->getType())) {
		return static_cast<MapTile*>(i);
	}
	return nullptr;
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
	Log::info(LOG_SERVER, "init player %i", player->getID());
	const MapSettingsMessage mapSettingsMsg(_settings, _startPositions.size());
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
		Log::info(LOG_SERVER, "* %s (waiting)", name.c_str());
	}
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const std::string& name = (*i)->getName();
		Log::info(LOG_SERVER, "* %s (spawned)", name.c_str());
	}
}

void Map::sendPlayersList () const
{
	std::vector<std::string> names;
	for (PlayerListConstIter i = _players.begin(); i != _players.end(); ++i) {
		const std::string& name = (*i)->getName();
		names.push_back(name);
	}
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

char Map::getSokobanFieldId (const IEntity *entity) const
{
	const EntityType& t = entity->getType();
	if (EntityTypes::isGround(t))
		return Sokoban::GROUND;
	if (EntityTypes::isTarget(t))
		return Sokoban::TARGET;
	// the order matters
	if (EntityTypes::isSolid(t))
		return Sokoban::WALL;
	if (EntityTypes::isPackage(t))
		return Sokoban::PACKAGE;
	if (EntityTypes::isPlayer(t))
		return Sokoban::PLAYER;
	return Sokoban::GROUND;
}

bool Map::setField (IEntity *entity, int col, int row)
{
	const int index = _state.getIndex(col, row);
	auto fi = _field[index];
	if (fi == nullptr) {
		_field[index] = entity;
	} else {
		// ground and target have low prio
		if (fi->isGround() || fi->isTarget())
			_field[index] = entity;
	}
	char nc = getSokobanFieldId(entity);
	if (_state.isInvalid(col, row)) {
		_state.clearFieldForIndex(index);
		_state.setFieldForIndex(index, nc);
		return true;
	}
	const char c = _state.getField(col, row);
	if (c == Sokoban::PLAYER) {
		if (nc == Sokoban::TARGET)
			nc = Sokoban::PLAYERONTARGET;
		else if (nc == Sokoban::GROUND)
			return true;
		else
			return false;
	} else if (c == Sokoban::PACKAGE) {
		if (nc == Sokoban::TARGET)
			nc = Sokoban::PACKAGEONTARGET;
		else if (nc == Sokoban::GROUND)
			return true;
		else
			return false;
	} else if (c == Sokoban::TARGET) {
		if (nc == Sokoban::PACKAGE)
			nc = Sokoban::PACKAGEONTARGET;
		else if (nc == Sokoban::PLAYER)
			nc = Sokoban::PLAYERONTARGET;
		else if (nc == Sokoban::GROUND)
			return true;
		else
			return false;
	}
	// we expect that there is already a value set, so we need to clear it first
	if (_state.clearField(col, row) == '\0')
		return false;
	return _state.setField(col, row, nc);
}

void Map::addEntity (int clientMask, const IEntity& entity) const
{
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const AddEntityMessage msg(entity.getID(), entity.getType(), Animation::NONE,
			entity.getSpriteID(), entity.getCol(), entity.getRow(), 1.0f, 1.0f, angle, ENTITY_ALIGN_UPPER_LEFT);
	_serviceProvider->getNetwork().sendToClients(clientMask, msg);
}

void Map::updateEntity (int clientMask, const IEntity& entity) const
{
	const EntityAngle angle = static_cast<EntityAngle>(RadiansToDegrees(entity.getAngle()));
	const UpdateEntityMessage msg(entity.getID(), entity.getCol(), entity.getRow(), angle, entity.getState());
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
	Log::error(LOG_MAP, "could not find the player with the clientId %i", clientId);
	return false;
}

bool Map::visitEntity (IEntity *entity)
{
	entity->update(Constant::DELTA_PHYSICS_MILLIS);
	return false;
}

void Map::rebuildField ()
{
	_field.assign(_width * _height, nullptr);
	_state.clearBoard();
	for (EntityListIter i = _entities.begin(); i != _entities.end(); ++i) {
		SDL_assert_always(setField(*i, (*i)->getCol(), (*i)->getRow()));
	}
	for (PlayerListIter i = _players.begin(); i != _players.end(); ++i) {
		SDL_assert_always(setField(*i, (*i)->getCol(), (*i)->getRow()));
	}
#ifdef DEBUG
	Log::trace(LOG_MAP, "State:\n%s", _state.toString().c_str());
#endif
}

void Map::autoStart () {
	// already spawned
	if (!_players.empty())
		return;
	// no players available yet
	if (_playersWaitingForSpawn.empty())
		return;
	// singleplayer already auto starts a map
	if (!_serviceProvider->getNetwork().isMultiplayer())
		return;
	// not enough players connected yet
	if (_playersWaitingForSpawn.size() < _startPositions.size())
		return;
	Log::info(LOG_SERVER, "starting the map");
	startMap();
}

void Map::sendDeadlocks(ClientId clientId)
{
	Log::info(LOG_SERVER, "Send deadlocks to client %i", (int)clientId);
	const DeadlockSet& deadlocks = _state.getDeadlockDetector().getDeadlocks();
	if (deadlocks.empty()) {
		Log::info(LOG_SERVER, "No deadlocks found");
		return;
	}
	std::vector<int> indices(deadlocks.size());
	for (int index : deadlocks) {
		indices.push_back(index);
	}
	const ShowDeadlocksMessage deadlocksmsg(_width, _height, indices);
	_serviceProvider->getNetwork().sendToClient(clientId, deadlocksmsg);
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

	if (_restartDue > 0 && _restartDue <= _time) {
		const std::string currentName = getName();
		Log::info(LOG_MAP, "restarting map %s", currentName.c_str());
		load(currentName);
	}
	handleAutoSolve(deltaTime);
}

void Map::handleAutoSolve (uint32_t deltaTime)
{
	if (!_autoSolve)
		return;

	if (_solution.empty()) {
		abortAutoSolve();
		Log::error(LOG_SERVER, "no solution");
		return;
	}

	_nextSolveStep -= deltaTime;
	if (_nextSolveStep > 0)
		return;

	_nextSolveStep = Config.getConfigVar("solvestepmillis", "250")->getIntValue();

	if (_solution[0] == '(') {
		abortAutoSolve();
		Log::error(LOG_SERVER, "x() repeat syntax is not supported");
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
		abortAutoSolve();
		Log::error(LOG_SERVER, "no player connected");
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
				Log::debug(LOG_SERVER, "remove player by visit %i: %s", e->getID(), e->getType().name.c_str());
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
				Log::debug(LOG_SERVER, "remove entity by visit %i: %s", e->getID(), e->getType().name.c_str());
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
}

void Map::init (IFrontend *frontend, ServiceProvider& serviceProvider)
{
	_frontend = frontend;
	_serviceProvider = &serviceProvider;
}

int Map::getMaxPlayers() const
{
	return _startPositions.size();
}

void Map::triggerPause ()
{
	if (!_serviceProvider->getNetwork().isServer())
		return;
	_pause ^= true;
	const PauseMessage msg(_pause);
	_serviceProvider->getNetwork().sendToAllClients(msg);
	if (_autoSolve && !_pause) {
		static const AutoSolveStartedMessage msgAutoSolveContinue;
		_serviceProvider->getNetwork().sendToAllClients(msgAutoSolveContinue);
	}
	Log::info(LOG_MAP, "pause: %s", (_pause ? "true" : "false"));
}

}
