#include "Camera.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Logger.h"
#include "engine/common/EventHandler.h"
#include "engine/common/IFrontend.h"

Camera::Camera () :
		_mapPixelWidth(0), _mapPixelHeight(0), _scrollingAreaWidth(0), _scrollingAreaHeight(0), _scale(0), _zoom(1.0f)
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
	_scrollingAreaWidth = std::max(0, mapGridWidth * _scale - _mapPixelWidth);
	_scrollingAreaHeight = std::max(0, mapGridHeight * _scale - _mapPixelHeight);
}

void Camera::update (const vec2& playerPos, Direction direction, float zoom)
{
	_zoom = zoom;
	if (_scrollingAreaWidth > 0) {
		_viewportX = -clamp(playerPos.x * _scale * _zoom - _mapPixelWidth / 2.0f * _zoom, 0.0f, _scrollingAreaWidth * _zoom);
	}
	if (_scrollingAreaHeight > 0) {
		_viewportY = -clamp(playerPos.y * _scale * _zoom - _mapPixelHeight / 2.0f * _zoom, 0.0f, _scrollingAreaHeight * _zoom);
	}
}
