#include "Player.h"
#include "cavepacker/server/map/Map.h"
#include "engine/common/Logger.h"
#include "engine/common/network/INetwork.h"
#include "engine/common/System.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/network/IProtocolHandler.h"

Player::Player (Map& map, ClientId clientId) :
		IEntity(EntityTypes::PLAYER, map, 0, 0), _clientId(clientId) {
	_solutionSave.reserve(256);
}

Player::~Player ()
{
}

void Player::storeStep (char step)
{
	_solutionSave += step;
}

bool Player::undo ()
{
	if (_solutionSave.empty())
		return false;
	std::string::reverse_iterator i = _solutionSave.rbegin();
	const char s = *i;
	_solutionSave.erase(_solutionSave.size() - 1);

	int xPlayer;
	int yPlayer;
	getOppositeXY(s, xPlayer, yPlayer);
	const int origCol = _col;
	const int origRow = _row;
	const int targetCol = origCol + xPlayer;
	const int targetRow = origRow + yPlayer;
	if (!setPos(targetCol, targetRow)) {
		debug(LOG_SERVER, "failed to undo a move of the player");
		return false;
	}
	if (tolower(s) != s) {
		int xPackage;
		int yPackage;
		getXY(s, xPackage, yPackage);
		const int packageCol = origCol + xPackage;
		const int packageRow = origRow + yPackage;
		if (!_map.undoPackage(packageCol, packageRow, origCol, origRow)) {
			setPos(origCol, origRow);
			return false;
		}
	}
	return true;
}
