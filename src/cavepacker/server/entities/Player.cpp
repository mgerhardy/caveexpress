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
	// move player and move touching packages
	MapTile* package = _map.getPackage(_col + x, _row + y);
	if (package != nullptr) {
		if (!_map.isFree(_col + x * 2, _row + y * 2)) {
			return false;
		}
		package->setPos(_col + x * 2, _row + y * 2);
	}
	_map.increaseMoves();
	return setPos(_col + x, _row + y);
}
