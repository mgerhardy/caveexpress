#pragma once

#include "common/Math.h"
#include "common/Direction.h"

class IFrontend;

class Camera {
private:
	// the dimensions of the area the map is rendered in (might be only a portion of the real map dimensions)
	int _mapPixelWidth;
	int _mapPixelHeight;

	// the full map grid width - in combination with the scale for one tile, you can get the full map size in pixels, too
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

	int _scrollOffsetX;
	int _scrollOffsetY;

	void reset ();

public:
	void init (int width, int height, int mapGridWidth, int mapGridHeight, int scale);
	void update (const vec2& playerPos, Direction direction, float zoom);
	void scroll (int offsetX, int offsetY);
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
