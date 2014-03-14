#include "Geyser.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/server/entities/modificators/WindModificator.h"

#define RANDOM_WAITING_DELAY 10000
#define MIN_WAITING_DELAY 3000
#define MIN_ACTIVE_TIME 3000
#define RANDOM_ACTIVE_TIME 5000

Geyser::Geyser (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY) :
		MapTile(map, spriteID, gridX, gridY, ThemeTypes::isIce(map.getTheme()) ? EntityTypes::GEYSER_ICE : EntityTypes::GEYSER_ROCK), _modificator(
				nullptr), _lastActivation(0), _activeTime(0)
{
	setAnimationType(Animations::ANIMATION_IDLE);
	_modificator = new WindModificator(_map, DIRECTION_UP, 11.0f, 2.0f);
	_modificator->createBody(getPos(), -0.5f);
	updateLastActivation();
}

Geyser::~Geyser ()
{
	delete _modificator;
}

inline void Geyser::updateLastActivation ()
{
	_lastActivation = rand() % RANDOM_WAITING_DELAY + MIN_WAITING_DELAY + _activeTime;
}

void Geyser::update (uint32_t deltaTime)
{
	MapTile::update(deltaTime);
	_modificator->update(deltaTime);

	if (_lastActivation < _time) {
		_activeTime = _time + rand() % RANDOM_ACTIVE_TIME + MIN_ACTIVE_TIME;
		updateLastActivation();
		// TODO: handle SpriteDefFrame::active
		_modificator->setModificatorState(true);
		setAnimationType(Animations::ANIMATION_ACTIVE);
	}
	if (_activeTime < _time) {
		_modificator->setModificatorState(false);
		setAnimationType(Animations::ANIMATION_IDLE);
	}
}
