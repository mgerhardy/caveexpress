#pragma once

#include "GameRoundData.h"
#include "IGameStatePersister.h"
#include "engine/common/IProgressCallback.h"
#include "engine/common/Math.h"
#include "engine/common/Logger.h"

class GameRound {
private:
	// all locations of the current round
	Locations _locations;
	Locations _allLocations;
	Location _currentLocation;
	IGameStatePersister& _persister;
	IProgressCallback *_callback;

	double getDistance (float long1, float lat1, float long2, float lat2) const;
public:
	GameRound (IGameStatePersister& persister, IProgressCallback *callback);
	virtual ~GameRound ();
	void init ();

	bool hasMoreLocations () const;
	int activateNextLocation ();

	// return the current location
	const Location& getLocation () const;
	// return all locations for the round
	const Locations& getLocations () const;

	// persist the current round state
	void save ();

	// getters for the current location
	float getLongitude () const;
	float getLatitude () const;

	// setters for the current location
	void setFailed (bool failed);
	void setTimeTaken (uint32_t time);
	void setCoordinates (float longitude, float latitude);
};

inline float GameRound::getLongitude () const
{
	return _currentLocation.longitude;
}

inline float GameRound::getLatitude () const
{
	return _currentLocation.latitude;
}

inline void GameRound::setFailed (bool failed)
{
	return _currentLocation.setFailed(failed);
}

inline void GameRound::setTimeTaken (uint32_t time)
{
	return _currentLocation.setTimeTaken(time);
}

inline double GameRound::getDistance (float long1, float lat1, float long2, float lat2) const
{
	info(LOG_CLIENT, "get distance: " + string::toString(long1) + " " + string::toString(lat1) + " " + string::toString(long2) + " " + string::toString(lat2));
	lat1 = DegreesToRadians(lat1);
	lat2 = DegreesToRadians(lat2);
	const double deltaLongitude = DegreesToRadians(long1 - long2);
	const double distance = acos(clamp(cos(lat1) * cos(lat2) * cos(deltaLongitude) + sin(lat1) * sin(lat2), -1.0, 1.0));
	const double conv = RadiansToDegrees(distance);
	return conv;
}

inline void GameRound::setCoordinates (float longitude, float latitude)
{
	const double distance = getDistance(longitude, latitude, getLongitude(), getLatitude());
	_currentLocation.setDistance(distance);
}

inline const Location& GameRound::getLocation () const
{
	return _currentLocation;
}

inline const Locations& GameRound::getLocations () const
{
	return _allLocations;
}
