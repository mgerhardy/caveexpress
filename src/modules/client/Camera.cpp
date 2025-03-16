#include "Camera.h"
#include "common/ConfigManager.h"
#include "common/Log.h"
#include "common/EventHandler.h"
#include "common/IFrontend.h"

Camera::Camera () :
		_mapPixelWidth(0), _mapPixelHeight(0), _mapGridWidth(0), _mapGridHeight(0), _scrollingAreaWidth(0), _scrollingAreaHeight(0), _scaleGridToPixel(0), _scrollOffsetX(0), _scrollOffsetY(0)
{
	reset();
}

Camera::~Camera ()
{
}

inline void Camera::reset ()
{
	_viewportX = 0;
	_viewportY = 0;
}

void Camera::init (int mapPixelWidth, int mapPixelHeight, int mapGridWidth, int mapGridHeight, int scale)
{
	reset();
	_scaleGridToPixel = scale;
	_mapPixelWidth = mapPixelWidth;
	_mapPixelHeight = mapPixelHeight;
	_mapGridWidth = mapGridWidth;
	_mapGridHeight = mapGridHeight;
	_scrollingAreaWidth = std::max(0, _mapGridWidth * _scaleGridToPixel - _mapPixelWidth);
	_scrollingAreaHeight = std::max(0, _mapGridHeight * _scaleGridToPixel - _mapPixelHeight);
	_scrollOffsetX = 0;
	_scrollOffsetY = 0;
}

void Camera::scroll (int offsetX, int offsetY)
{
	_scrollOffsetX += offsetX;
	_scrollOffsetY += offsetY;

	const int w = _mapPixelWidth / 2;
	const int h = _mapPixelHeight / 2;
	_scrollOffsetX = clamp(_scrollOffsetX, -w, w);
	_scrollOffsetY = clamp(_scrollOffsetY, -h, h);
}

bool Camera::update (const vec2& playerPos, Direction direction, float zoom)
{
	// TODO: don't scroll on every pixel - but only if the player is near the border
	// after zooming center the map
	const int pixelW = _mapGridWidth * _scaleGridToPixel * zoom;
	const int nodeW = _mapPixelWidth;
	const int oldViewX = _viewportX;
	const int oldViewY = _viewportY;
	if (pixelW < nodeW) {
		// if we can show the full width of the map - then center it
		_viewportX = _mapPixelWidth / 2 - pixelW / 2;
	} else {
		// TODO: broken - doesn't center on the player
		_viewportX = -clamp(playerPos.x * _scaleGridToPixel - _mapPixelWidth / 2.0f, 0.0f, static_cast<float>(_scrollingAreaWidth)) * zoom;
	}
	const int pixelH = _mapGridHeight * _scaleGridToPixel * zoom;
	const int nodeH = _mapPixelHeight;
	if (pixelH < nodeH) {
		// if we can show the full width of the map - then center it
		_viewportY = _mapPixelHeight / 2 - pixelH / 2;
	} else {
		// TODO: broken - doesn't center on the player
		_viewportY = -clamp(playerPos.y * _scaleGridToPixel - _mapPixelHeight / 2.0f, 0.0f, static_cast<float>(_scrollingAreaHeight)) * zoom;
	}
	_viewportX += _scrollOffsetX;
	_viewportY += _scrollOffsetY;
	Log::trace(LOG_CLIENT, "zoom: %f, viewportX %i, pixelW %i, nodeW: %i", zoom, _viewportX, pixelW, nodeW);
	return oldViewX != _viewportX || oldViewY != _viewportY;
}
