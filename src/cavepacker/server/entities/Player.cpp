#include "Player.h"
#include "cavepacker/server/map/Map.h"
#include "common/Log.h"
#include "network/INetwork.h"
#include "common/System.h"
#include "common/ConfigManager.h"
#include "network/IProtocolHandler.h"
#include <SDL.h>

namespace cavepacker {

Player::Player (Map& map, ClientId clientId) :
		IEntity(EntityTypes::PLAYER, map, 0, 0), _clientId(clientId), _targetIndex(NO_TARGET_INDEX), _lastStep(0u) {
	_solutionSave.reserve(256);
}

Player::~Player ()
{
}

void Player::update (uint32_t deltaTime) {
	IEntity::update(deltaTime);
	if (_targetIndex == NO_TARGET_INDEX) {
		return;
	}

	if (_map.isAt(this, _targetIndex)) {
		_targetIndex = NO_TARGET_INDEX;
		return;
	}

	if (_time - _lastStep < 250u) {
		return;
	}
	_lastStep = _time;

	int currentPos = _map.getPositionIndex(this);
	const char dir = _map.getDirectionForMove(currentPos, _targetIndex);
	if (dir == '\0') {
		_targetIndex = NO_TARGET_INDEX;
		return;
	}
	if (!_map.movePlayer(this, dir)) {
		_targetIndex = NO_TARGET_INDEX;
	}
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

	setTargetIndex(NO_TARGET_INDEX);

	int xPlayer;
	int yPlayer;
	getOppositeXY(s, xPlayer, yPlayer);
	const int origCol = _col;
	const int origRow = _row;
	const int targetCol = origCol + xPlayer;
	const int targetRow = origRow + yPlayer;
	if (!setPos(targetCol, targetRow)) {
		Log::debug(LOG_SERVER, "failed to undo a move of the player");
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

void Player::setTargetIndex(int index)
{
	SDL_assert_always(_targetIndex < _map.getMapWidth() * _map.getMapHeight());
	SDL_assert_always(_targetIndex >= -1);
	_targetIndex = index;
}

}
