#include "Camera.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/Logger.h"
#include "engine/common/EventHandler.h"
#include "engine/common/IFrontend.h"

Camera::Camera () :
		_mapPixelWidth(0), _mapPixelHeight(0), _mapGridWidth(0), _mapGridHeight(0), _scrollingAreaWidth(0), _scrollingAreaHeight(0), _scale(0), _zoom(1.0f)
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
}

void Camera::update (const vec2& playerPos, Direction direction, float zoom)
{
	_zoom = zoom;
	_scrollingAreaWidth = std::max(0, _mapGridWidth * _scale - _mapPixelWidth) * _zoom;
	if (_scrollingAreaWidth > 0) {
		_viewportX = -clamp(playerPos.x * _scale - _mapPixelWidth / 2.0f, 0.0f, static_cast<float>(_scrollingAreaWidth)) * _zoom;
	}
	_scrollingAreaHeight = std::max(0, _mapGridHeight * _scale - _mapPixelHeight) * _zoom;
	if (_scrollingAreaHeight > 0) {
		_viewportY = -clamp(playerPos.y * _scale - _mapPixelHeight / 2.0f, 0.0f, static_cast<float>(_scrollingAreaHeight)) * _zoom;
	}
}
