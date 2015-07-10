#include "Player.h"
#include "miniracer/server/map/Map.h"
#include "common/Log.h"
#include "network/INetwork.h"
#include "common/System.h"
#include "common/ConfigManager.h"
#include "network/IProtocolHandler.h"

namespace miniracer {

Player::Player (Map& map, ClientId clientId) :
		IEntity(EntityTypes::PLAYER, map), _clientId(clientId) {
}

Player::~Player ()
{
}

}
