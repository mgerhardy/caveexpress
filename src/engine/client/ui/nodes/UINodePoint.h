#pragma once

#include "UINodeLabel.h"

class UINodePoint: public UINodeLabel {
private:
	int _current;
	int _points;
	uint32_t _lastUpdate;
	uint32_t _updateDelay;
public:
	UINodePoint (IFrontend *frontend, uint32_t updateDelay = 10) :
			UINodeLabel(frontend, "0", getFont(HUGE_FONT)), _current(-1), _points(0), _lastUpdate(0), _updateDelay(
					updateDelay)
	{
		Vector4Set(colorWhite, _fontColor);
	}

	void update (uint32_t deltaTime) override
	{
		UINodeLabel::update(deltaTime);
		if (_current == _points)
			return;

		const uint32_t passed = _time - _lastUpdate;
		if (passed < _updateDelay)
			return;

		_lastUpdate += _updateDelay;
		if (_current > _points)
			--_current;
		else
			++_current;
		setLabel(string::toString(_current));
	}

	// this will increase the points over time. The given points value is an absolute value
	void increasePoints (int points)
	{
		_points = points;
	}

	// this will increase the points over time. The given points value is a relative value
	void addPoints (int points)
	{
		_points += points;
	}

	// this sets the points and starts the increasing from the beginning. The give points value is an absolute value
	void setPoints (int points)
	{
		_current = 0;
		_points = points;
	}
};
