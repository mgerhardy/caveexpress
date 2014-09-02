#include "RoundController.h"
#include "geophoto/client/ui/nodes/UINodeMap.h"
#include "engine/client/ui/nodes/UINodeLabel.h"
#include "engine/client/ui/nodes/UINodeBar.h"
#include "engine/common/Logger.h"

#define PICTURE_SHOW_MILLIS 6000
#define BAR_TURN_RED_MILLIS (PICTURE_SHOW_MILLIS / 2)
#define SHOW_RESULT_DELAY_MILLIS 3000
#define EARTH_FADE_IN_MILLIS 300

RoundController::RoundController (IGameStatePersister& persister, IProgressCallback *callback) :
		_round(persister, callback), _map(nullptr), _timeBar(nullptr), _timeNode(nullptr), _time(0), _state(0), _pictureTime(
				0), _clickedTime(0), _clickedScreenX(0), _clickedScreenY(0), _screenCoordX(0), _screenCoordY(0)
{
}

RoundController::~RoundController ()
{
}

void RoundController::init (UINodeMap *map, UINodeBar* timeBar, UINodeLabel* timeNode)
{
	_map = map;
	_timeBar = timeBar;
	_timeNode = timeNode;
}

void RoundController::update (uint32_t deltaTime)
{
	_time += deltaTime;

	if (_state == STATE_PREPARE) {
		handleStateChange(STATE_LOADING);
	} else if (_state == STATE_LOADING) {
		if (_map->fadeIn(deltaTime, EARTH_FADE_IN_MILLIS))
			handleStateChange(STATE_PICTURE);
	} else if (_state == STATE_FADEIN_PHOTO) {
		handleStateChange(STATE_PICTURE);
	} else if (_state == STATE_FADEIN_EARTH) {
		if (_map->fadeIn(deltaTime, EARTH_FADE_IN_MILLIS))
			handleStateChange(STATE_WAITING);
	} else if (_state == STATE_PICTURE || _state == STATE_WAITING) {
		const int32_t c = _timeBar->getMax() - (_time - _pictureTime);
		if (c <= 0) {
			handleStateChange(STATE_FAILED);
		} else {
			_timeBar->setCurrent(c);
			if (c <= BAR_TURN_RED_MILLIS) {
				_timeBar->setBarColor(colorRed);
				_timeNode->setVisible(true);
				_timeNode->setLabel(string::toString(1 + c / 1000));
			}
		}
	}
}

void RoundController::handleStateChange (int newState)
{
	_state = newState;

	switch (_state) {
	case STATE_PREPARE: {
		_map->setLoading();
		_timeBar->setMax(PICTURE_SHOW_MILLIS);
		_round.init();
		break;
	}
	case STATE_LOADING: {
		const int id = _round.activateNextLocation();
		if (id == -1) {
			handleStateChange(STATE_ROUNDEND);
			return;
		}
		_timeNode->setVisible(false);
		_map->setImage(string::toString(id));
		_map->setAlpha(0.01f);
		break;
	}
	case STATE_PICTURE: {
		_timeBar->setVisible(true);
		_timeBar->setBarColor(colorGreen);
		_timeBar->setCurrent(_timeBar->getMax());
		_pictureTime = _time;
		break;
	}
	case STATE_WAITING: {
		break;
	}
	case STATE_FADEIN_EARTH: {
		_map->setEarth();
		_map->setAlpha(0.01f);
		convertToScreen(_round.getLongitude(), _round.getLatitude(), _screenCoordX, _screenCoordY);
		break;
	}
	case STATE_CLICKED: {
		_timeBar->setVisible(false);
		_timeNode->setVisible(false);
		break;
	}
	case STATE_FAILED: {
		_map->setEarth();
		_timeBar->setVisible(false);
		_timeNode->setVisible(false);
		_round.setFailed(true);
		break;
	}
	default: {
		break;
	}
	}
}

void RoundController::convertToMap (int x, int y, float &longitude, float& latitude) const
{
	info(LOG_CLIENT, "convert to map: " + string::toString(x) + " " + string::toString(y) + " " + string::toString(_map->getRenderX()) + " " + string::toString(_map->getRenderWidth()));
	longitude = ((_map->getRenderX() - x) / static_cast<float>(_map->getRenderWidth()) + 0.5f) * -360.0f;
	latitude = ((_map->getRenderY() - y) / static_cast<float>(_map->getRenderHeight()) + 0.5f) * 180.0f;

	while (longitude > 180.0f)
		longitude -= 360.0f;
	while (latitude < -180.0f)
		latitude += 360.0f;
}

void RoundController::convertToScreen (float longitude, float latitude, int &x, int &y) const
{
	const float lx = longitude / 360.0f;
	const float ly = latitude / 180.0f;
	x = _map->getRenderX() + 0.5f * _map->getRenderWidth() + lx * _map->getRenderWidth();
	y = _map->getRenderY() + 0.5f * _map->getRenderHeight() - ly * _map->getRenderHeight();
}

void RoundController::handleClick (int x, int y)
{
	switch (_state) {
	case STATE_PICTURE:
		handleStateChange(STATE_FADEIN_EARTH);
		break;
	case STATE_CLICKED:
		handleStateChange(STATE_LOADING);
		break;
	case STATE_FAILED:
		handleStateChange(STATE_CLICKED);
		_round.save();
		break;
	case STATE_WAITING:
		_clickedScreenX = x;
		_clickedScreenY = y;
		_clickedTime = _time;
		handleStateChange(STATE_CLICKED);

		float clickedLong, clickedLat;
		convertToMap(_clickedScreenX, _clickedScreenY, clickedLong, clickedLat);
		_round.setCoordinates(clickedLong, clickedLat);
		_round.setTimeTaken(_clickedTime - _pictureTime);
		_round.save();
		break;
	case STATE_ROUNDEND:
		handleStateChange(STATE_END);
		break;
	default:
		break;
	}
}
