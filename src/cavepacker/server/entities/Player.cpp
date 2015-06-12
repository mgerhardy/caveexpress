#include "Player.h"
#include "cavepacker/server/map/Map.h"
#include "common/Logger.h"
#include "network/INetwork.h"
#include "common/System.h"
#include "common/ConfigManager.h"
#include "network/IProtocolHandler.h"

namespace cavepacker {

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
	// we moved a package with this step
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
	_solutionSave.erase(_solutionSave.size() - 1);
	return true;
}

}
