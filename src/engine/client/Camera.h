#pragma once

#include "engine/common/Math.h"
#include "engine/common/Direction.h"

class IFrontend;

class Camera {
private:
	int _mapPixelWidth;
	int _mapPixelHeight;

	int _mapGridWidth;
	int _mapGridHeight;

	int _viewportX;
	int _viewportY;

	// this is the amount of pixels that can be scrolled in a map
	// in other words, the amount of pixels that are missing to display the full width of the map
	int _scrollingAreaWidth;

	// this is the amount of pixels that can be scrolled in a map
	// in other words, the amount of pixels that are missing to display the full height of the map
	int _scrollingAreaHeight;

	// the pixels of one grid tile
	int _scale;

	float _zoom;

	void reset ();

public:
	void init (int width, int height, int mapGridWidth, int mapGridHeight, int scale);
	void update (const vec2& playerPos, Direction direction, float zoom);
	int getViewportX () const;
	int getViewportY () const;

	Camera ();
	~Camera ();
};

inline int Camera::getViewportX () const
{
	return _viewportX;
}

inline int Camera::getViewportY () const
{
	return _viewportY;
}
