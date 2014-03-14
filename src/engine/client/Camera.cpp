#include "Camera.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Logger.h"
#include "engine/common/EventHandler.h"
#include "engine/common/IFrontend.h"

Camera::Camera () :
		_mapPixelWidth(0), _mapPixelHeight(0), _scrollingAreaWidth(0), _scrollingAreaHeight(0), _scale(0)
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

void Camera::update (const vec2& playerPos, Direction direction)
{
	if (_scrollingAreaWidth > 0) {
		_viewportX = -clamp(static_cast<int>(playerPos.x * _scale - _mapPixelWidth / 2), 0, _scrollingAreaWidth);
	}
	if (_scrollingAreaHeight > 0) {
		_viewportY = -clamp(static_cast<int>(playerPos.y * _scale - _mapPixelHeight / 2), 0, _scrollingAreaHeight);
	}
}
