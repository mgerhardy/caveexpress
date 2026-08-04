#include "Player.h"
#include "cavepacker/server/map/Map.h"
#include "common/Log.h"
#include "network/INetwork.h"
#include "common/System.h"
#include "common/ConfigManager.h"
#include "network/IProtocolHandler.h"
#include <SDL.h>
#include <algorithm>

namespace cavepacker {

Player::Player (Map& map, ClientId clientId) :
		IEntity(EntityTypes::PLAYER, map, 0, 0), _clientId(clientId), _targetIndex(NO_TARGET_INDEX), _lastStep(0u),
		_heldDirection(0) {
	_solutionSave.reserve(256);
}

Player::~Player ()
{
}

char Player::getHeldMoveStep () const
{
	if (_heldDirection & DIRECTION_UP)
		return MOVE_UP;
	if (_heldDirection & DIRECTION_DOWN)
		return MOVE_DOWN;
	if (_heldDirection & DIRECTION_LEFT)
		return MOVE_LEFT;
	if (_heldDirection & DIRECTION_RIGHT)
		return MOVE_RIGHT;
	return '\0';
}

void Player::setHeldDirection (Direction dir)
{
	if (dir & DIRECTION_HORIZONTAL)
		_heldDirection &= ~DIRECTION_HORIZONTAL;
	if (dir & DIRECTION_VERTICAL)
		_heldDirection &= ~DIRECTION_VERTICAL;
	_heldDirection |= dir;
	// MovementHandler already applies the first step; delay the next auto-step.
	_lastStep = _time;
}

void Player::clearHeldDirection (Direction dir)
{
	_heldDirection &= ~dir;
}

void Player::update (uint32_t deltaTime) {
	IEntity::update(deltaTime);
	if (_map.isDone() || _map.isFailed() || _map.isPause()) {
		_heldDirection = 0;
		_targetIndex = NO_TARGET_INDEX;
		return;
	}

	if (_targetIndex != NO_TARGET_INDEX) {
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
		return;
	}

	// Keep walking while a direction key/button stays pressed.
	const char step = getHeldMoveStep();
	if (step == '\0')
		return;

	const uint32_t repeatMs = std::max(100u,
			static_cast<uint32_t>(Config.getConfigVar("clientmovelerpmillis", "200")->getIntValue()));
	if (_time - _lastStep < repeatMs)
		return;
	_lastStep = _time;
	_map.movePlayer(this, step);
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
	_heldDirection = 0;

	int xPlayer;
	int yPlayer;
	getOppositeXY(s, xPlayer, yPlayer);
	const int origCol = _col;
	const int origRow = _row;
	const int targetCol = origCol + xPlayer;
	const int targetRow = origRow + yPlayer;
	if (!setPos(targetCol, targetRow)) {
		Log::debug(LOG_GAMEIMPL, "failed to undo a move of the player");
		return false;
	}
	_map.rebuildField();

	// Uppercase steps are package pushes (sokoban standard).
	if (tolower(s) != s) {
		int xPackage;
		int yPackage;
		getXY(s, xPackage, yPackage);
		const int packageCol = origCol + xPackage;
		const int packageRow = origRow + yPackage;
		if (!_map.undoPackage(packageCol, packageRow, origCol, origRow)) {
			setPos(origCol, origRow);
			_map.rebuildField();
			return false;
		}
	}
	_solutionSave.erase(_solutionSave.size() - 1);
	return true;
}

void Player::setTargetIndex(int index)
{
	SDL_assert_always(_map.getMapWidth() > 0);
	SDL_assert_always(_map.getMapHeight() > 0);
	SDL_assert_always(_targetIndex < _map.getMapWidth() * _map.getMapHeight());
	SDL_assert_always(_targetIndex >= -1);
	_targetIndex = index;
	if (index != NO_TARGET_INDEX)
		_heldDirection = 0;
}

}
