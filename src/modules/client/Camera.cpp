#include "Camera.h"
#include "common/Log.h"

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

void Camera::init (int mapPixelWidth, int mapPixelHeight, int mapGridWidth, int mapGridHeight, int scale, float zoom)
{
	_scaleGridToPixel = scale;
	_mapPixelWidth = mapPixelWidth;
	_mapPixelHeight = mapPixelHeight;
	_mapGridWidth = mapGridWidth;
	_mapGridHeight = mapGridHeight;
	_scrollingAreaWidth = std::max(0, _mapGridWidth * _scaleGridToPixel - _mapPixelWidth);
	_scrollingAreaHeight = std::max(0, _mapGridHeight * _scaleGridToPixel - _mapPixelHeight);
	_scrollOffsetX = 0;
	_scrollOffsetY = 0;
	// Center maps that fit the view immediately. reset() left the viewport at 0,0, so the
	// first rendered frame was in the upper-left until update() ran with a player.
	update(vec2_zero, 0, zoom);
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

static int viewportForAxis (float mapPx, float nodePx, float playerGrid, float scaledTile)
{
	if (mapPx <= nodePx) {
		return static_cast<int>((nodePx - mapPx) * 0.5f);
	}
	const float playerPx = playerGrid * scaledTile;
	const float desired = nodePx * 0.5f - playerPx;
	const float minVp = nodePx - mapPx;
	return static_cast<int>(clamp(desired, minVp, 0.0f));
}

bool Camera::update (const vec2& playerPos, Direction direction, float zoom)
{
	const float scaledTile = static_cast<float>(_scaleGridToPixel) * zoom;
	const float mapPixelW = static_cast<float>(_mapGridWidth) * scaledTile;
	const float mapPixelH = static_cast<float>(_mapGridHeight) * scaledTile;
	const float nodeW = static_cast<float>(_mapPixelWidth);
	const float nodeH = static_cast<float>(_mapPixelHeight);
	const int oldViewX = _viewportX;
	const int oldViewY = _viewportY;

	_scrollingAreaWidth = std::max(0, static_cast<int>(mapPixelW - nodeW));
	_scrollingAreaHeight = std::max(0, static_cast<int>(mapPixelH - nodeH));

	_viewportX = viewportForAxis(mapPixelW, nodeW, playerPos.x, scaledTile);
	_viewportY = viewportForAxis(mapPixelH, nodeH, playerPos.y, scaledTile);
	_viewportX += _scrollOffsetX;
	_viewportY += _scrollOffsetY;
	Log::trace(LOG_CLIENT, "zoom: %f, viewportX %i, mapW %f, nodeW: %f", zoom, _viewportX, mapPixelW, nodeW);
	return oldViewX != _viewportX || oldViewY != _viewportY;
}
