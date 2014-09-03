#pragma once

#include <string>
#include <stdint.h>

class Location {
private:
	// the amount of time between the picture showed and the user has chosen its location
	uint32_t _timeTaken;
	// used to remember whether a user ran out of time
	bool _failed;
	double _distance;

public:
	int id;
	std::string url;
	std::string thumbUrl;
	std::string location;
	std::string creator;
	float longitude;
	float latitude;
	int score;

	Location () :
			_timeTaken(0), _failed(false), _distance(0.0), id(0), url(""), thumbUrl(""), location(""), creator(""), longitude(
					0.0f), latitude(0.0f), score(0)
	{
	}

	void setFailed (bool failed);
	void setTimeTaken (uint32_t time);
	void setDistance (double distance);
	int getPoints () const;
	double getDistance () const;
	uint32_t getTimeTaken () const;
};

inline void Location::setFailed (bool failed)
{
	_failed = failed;
}

inline void Location::setTimeTaken (uint32_t time)
{
	_timeTaken = time;
}

inline void Location::setDistance (double distance)
{
	_distance = distance;
}

inline int Location::getPoints () const
{
	const int points = std::max(0.0, 100.0 - (_distance * _distance) / 100.0);
	return points;
}

inline double Location::getDistance () const
{
	return _distance;
}

inline uint32_t Location::getTimeTaken () const
{
	return _timeTaken;
}
