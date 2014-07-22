#include "Player.h"
#include "cavepacker/server/map/Map.h"
#include "engine/common/Logger.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/System.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/network/IProtocolHandler.h"

Player::Player (Map& map, ClientId clientId, int col, int row) :
		IEntity(EntityTypes::PLAYER, map, col, row), _clientId(clientId) {
}

Player::~Player ()
{
}

bool Player::move (int x, int y)
{
	info(LOG_SERVER, String::format("move player %i:%i", x, y));
	// move player and move touching packages
	const int targetCol = _col + x;
	const int targetRow = _row + y;
	MapTile* package = _map.getPackage(targetCol, targetRow);
	if (package != nullptr) {
		const int pCol = _col + x * 2;
		const int pRow = _row + y * 2;
		if (!_map.isFree(pCol, pRow)) {
			info(LOG_SERVER, "can't move here - can't move package. target field is blocked");
			return false;
		}
		if (!package->setPos(pCol, pRow)) {
			info(LOG_SERVER, "failed to move the package - thus can't move the player");
			return false;
		}
		info(LOG_SERVER, "moved package");
	}

	if (!_map.isFree(targetCol, targetRow)) {
		info(LOG_SERVER, "target field is blocked");
		return false;
	}
	if (!setPos(targetCol, targetRow)) {
		info(LOG_SERVER, "failed to move the player");
		return false;
	}

	_map.increaseMoves();
	return true;
}
