#include "Camera.h"
#include "common/ConfigManager.h"
#include "common/Logger.h"
#include "common/EventHandler.h"
#include "common/IFrontend.h"

Camera::Camera () :
		_mapPixelWidth(0), _mapPixelHeight(0), _mapGridWidth(0), _mapGridHeight(0), _scrollingAreaWidth(0), _scrollingAreaHeight(0), _scale(0), _scrollOffsetX(0), _scrollOffsetY(0)
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
	_scale = scale;
	_mapPixelWidth = mapPixelWidth;
	_mapPixelHeight = mapPixelHeight;
	_mapGridWidth = mapGridWidth;
	_mapGridHeight = mapGridHeight;
	_scrollingAreaWidth = std::max(0, _mapGridWidth * _scale - _mapPixelWidth);
	_scrollingAreaHeight = std::max(0, _mapGridHeight * _scale - _mapPixelHeight);
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

void Camera::update (const vec2& playerPos, Direction direction, float zoom)
{
	// after zooming center the map
	const int pixelW = _mapGridWidth * _scale * zoom;
	const int nodeW = _mapPixelWidth;
	if (pixelW < nodeW) {
		// if we can show the full width of the map - then center it
		_viewportX = _mapPixelWidth / 2 - pixelW / 2;
	} else {
		// TODO: broken - doesn't center on the player
		_viewportX = -clamp(playerPos.x * _scale - _mapPixelWidth / 2.0f, 0.0f, static_cast<float>(_scrollingAreaWidth)) * zoom;
	}
	const int pixelH = _mapGridHeight * _scale * zoom;
	const int nodeH = _mapPixelHeight;
	if (pixelH < nodeH) {
		// if we can show the full width of the map - then center it
		_viewportY = _mapPixelHeight / 2 - pixelH / 2;
	} else {
		// TODO: broken - doesn't center on the player
		_viewportY = -clamp(playerPos.y * _scale - _mapPixelHeight / 2.0f, 0.0f, static_cast<float>(_scrollingAreaHeight)) * zoom;
	}
	_viewportX += _scrollOffsetX;
	_viewportY += _scrollOffsetY;
	debug(LOG_CLIENT, String::format("zoom: %f, viewportX %i, pixelW %i, nodeW: %i", zoom, _viewportX, pixelW, nodeW));
}
