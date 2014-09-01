#pragma once

#include "client/round/GameRound.h"

class UINodeMap;
class UINodeBar;
class UINodeLabel;

// loads the json stuff and prepare the round
#define STATE_PREPARE  		1
// loads the image from the json data
#define STATE_LOADING  		2
// waiting for the location picture to fade in
#define STATE_FADEIN_PHOTO	3
// show the location picture
#define STATE_PICTURE  		4
// waiting for the earth image to fade in
#define STATE_FADEIN_EARTH  5
// waiting for the user to click the location
#define STATE_WAITING  		6
// user clicked - next location will be used in a few seconds
#define STATE_CLICKED  		7
// user didn't make a guess in the time frame
#define STATE_FAILED   		8
// all locations in the round where handled
#define STATE_ROUNDEND 		9
// waiting for the user to click on round overview
#define STATE_END 			10

class RoundController {
private:
	GameRound _round;
	UINodeMap *_map;
	UINodeBar *_timeBar;
	UINodeLabel *_timeNode;
	uint32_t _time;
	// the current state the node is in
	int _state;
	// the timestamp when the picture was shown
	uint32_t _pictureTime;
	// the timestamp the user had chosen a location on the screen
	uint32_t _clickedTime;
	// the screen coordinates the user clicked onto
	int _clickedScreenX;
	int _clickedScreenY;
	// current screen coordinates of the current location
	int _screenCoordX;
	int _screenCoordY;

	void convertToMap (int x, int y, float &longitude, float& latitude) const;
	void convertToScreen (float longitude, float latitude, int &x, int &y) const;

public:
	RoundController (IGameStatePersister& persister, IProgressCallback *callback);
	virtual ~RoundController ();

	void init (UINodeMap *map, UINodeBar* timeBar, UINodeLabel* timeNode);
	void update (uint32_t deltaTime);
	void handleStateChange (int newState);
	void handleClick (int x, int y);
	bool isClickAllowed () const;
	int getState () const;

	int getClickedScreenX () const;
	int getClickedScreenY () const;

	int getLocationScreenX () const;
	int getLocationScreenY () const;

	const Location& getCurrentLocation () const;

	const GameRound& getGameRound () const;
};

inline bool RoundController::isClickAllowed () const
{
	return _state == STATE_WAITING || _state == STATE_PICTURE || _state == STATE_FAILED || _state == STATE_ROUNDEND || _state == STATE_CLICKED;
}

inline int RoundController::getState () const
{
	return _state;
}

inline int RoundController::getClickedScreenX () const
{
	return _clickedScreenX;
}

inline int RoundController::getClickedScreenY () const
{
	return _clickedScreenY;
}

inline int RoundController::getLocationScreenX () const
{
	return _screenCoordX;
}

inline int RoundController::getLocationScreenY () const
{
	return _screenCoordY;
}

inline const Location& RoundController::getCurrentLocation () const
{
	return _round.getLocation();
}

inline const GameRound& RoundController::getGameRound () const
{
	return _round;
}
