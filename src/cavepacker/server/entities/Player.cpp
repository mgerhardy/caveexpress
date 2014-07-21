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
