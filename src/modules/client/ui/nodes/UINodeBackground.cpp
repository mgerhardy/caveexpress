#include "UINodeBackground.h"
#include "common/IFrontend.h"

UINodeBackground::UINodeBackground (IFrontend *frontend, const std::string& title, bool showVehicle) :
		UINode(frontend), _showVehicle(showVehicle), _title(title)
{
	setPadding(getScreenPadding());
	setSize(1.0, 1.0f);
	setBackgroundColor(colorBlack);
	_cave = loadTexture("ui-scene-cave-ice");
	_vehicle = loadTexture("ui-scene-player");
	_caveArt = loadTexture("ui-scene-caveart-ice");
	_imageWidth = _cave->getWidth();
	_imageHeight = _cave->getHeight();
	_amountHorizontal = _imageWidth <= 0 ? 1 : getRenderWidth(false) / _imageWidth + 1;
	_amountVertical = _imageHeight <= 0 ? 1 : getRenderHeight(false) / _imageHeight + 1;

	_tiles.push_back(loadTexture("ui-scene-tile1-ice"));
	_tiles.push_back(loadTexture("ui-scene-tile2-ice"));

	_font = getFont(LARGE_FONT);
	Vector4Set(colorWhite, _fontColor);
	_textWidth = _font->getTextWidth(_title);
	_textHeight = _font->getTextHeight(_title);
}

void UINodeBackground::renderMiddle(int x, int y) const {
	const int renderHeight = getRenderHeight(false);
	renderImage(getCave(), x + getRenderX(false), y + getRenderY(false) + renderHeight - _imageHeight);
	if (_amountVertical > 1)
		renderImage(getCaveArt(), x + getRenderX(false), y + getRenderY(false) + renderHeight - 2 * _imageHeight);

	const int tileCnt = _tiles.size();
	for (int row = 3; row <= _amountVertical; ++row) {
		renderImage(_tiles[(row * _amountHorizontal) % tileCnt],
				x + getRenderX(false), y + getRenderY(false) + renderHeight - row * _imageHeight);
	}

	for (int row = 1; row <= _amountVertical; ++row) {
		for (int col = 1; col < _amountHorizontal; ++col) {
			renderImage(_tiles[((row * _amountHorizontal) + col) % tileCnt],
					x + getRenderX(false) + _imageWidth * col,
					y + getRenderY(false) + renderHeight - row * _imageHeight);
		}
	}
	if (_showVehicle && _amountHorizontal > 1)
		renderImage(_vehicle, x + getRenderX(false) + _imageWidth, y + getRenderY(false) + renderHeight - _vehicle->getHeight());

	if (!_title.empty()) {
		x += getRenderCenterX() - _textWidth / 2;
		y += getRenderY() + 10;

		_font->print(_title, _fontColor, x, y);
	}
}
