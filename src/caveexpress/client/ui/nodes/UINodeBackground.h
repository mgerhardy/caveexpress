#pragma once

#include "engine/client/ui/nodes/UINode.h"
#include "engine/common/String.h"
#include "engine/client/ui/BitmapFont.h"
#include "engine/common/Payment.h"
#include "engine/common/Config.h"

class UINodeBackground: public UINode {
protected:
	TexturePtr _cave;
	TexturePtr _caveArt;
	TexturePtr _vehicle;

	bool _showVehicle;

	int _imageWidth;
	int _imageHeight;
	int _amountHorizontal;
	int _amountVertical;

	std::vector<TexturePtr> _tiles;
	const std::string _title;
	BitmapFontPtr _font;
	Color _fontColor;
	int _textWidth;
	int _textHeight;
public:
	UINodeBackground (IFrontend *frontend, const std::string& title, bool showVehicle = true) :
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
		_amountHorizontal = getRenderWidth(false) / _imageWidth + 1;
		_amountVertical = getRenderHeight(false) / _imageHeight + 1;

		_tiles.push_back(loadTexture("ui-scene-tile1-ice"));
		_tiles.push_back(loadTexture("ui-scene-tile2-ice"));

		_font = getFont(LARGE_FONT);
		Vector4Set(colorWhite, _fontColor);
		_textWidth = _font->getTextWidth(_title);
		_textHeight = _font->getTextHeight(_title);
	}

	virtual ~UINodeBackground ()
	{
	}

	inline void setAmountHorizontal (int amount)
	{
		setSize(1.0, 1.0f);
		setSize(amount * _imageWidth / static_cast<float>(getRenderWidth()), getHeight());
		_amountHorizontal = amount - 1;
	}

	inline void setAmountVertical (int amount)
	{
		setSize(1.0, 1.0f);
		setSize(getWidth(), amount * _imageHeight / static_cast<float>(getRenderHeight()));
		_amountVertical = amount - 1;
	}

	inline void setAmount (int amountHorizontal, int amountVertical)
	{
		setSize(1.0, 1.0f);
		setSize(amountHorizontal * _imageWidth / static_cast<float>(getRenderWidth()), amountVertical * _imageHeight / static_cast<float>(getRenderHeight()));
		_amountHorizontal = amountHorizontal - 1;
		_amountVertical = amountVertical - 1;
	}

	virtual TexturePtr getCave () const
	{
		return _cave;
	}

	virtual TexturePtr getCaveArt () const
	{
		return _caveArt;
	}

	virtual void render (int x, int y) const override
	{
		UINode::render(x, y);

		const int renderHeight = getRenderHeight(false);
		renderImage(getCave(), x + getRenderX(false), y + getRenderY(false) + renderHeight - _imageHeight);
		renderImage(getCaveArt(), x + getRenderX(false), y + getRenderY(false) + renderHeight - 2 * _imageHeight);

		const int tileCnt = _tiles.size();
		for (int row = 3; row <= _amountVertical; ++row) {
			renderImage(_tiles[(row * _amountHorizontal) % tileCnt], x + getRenderX(false),
					y + getRenderY(false) + renderHeight - row * _imageHeight);
		}

		for (int row = 1; row <= _amountVertical; ++row) {
			for (int col = 1; col <= _amountHorizontal; ++col) {
				renderImage(_tiles[((row * _amountHorizontal) + col) % tileCnt], x + getRenderX(false) + _imageWidth * col,
						y + getRenderY(false) + renderHeight - row * _imageHeight);
			}
		}
		if (_showVehicle)
			renderImage(_vehicle, x + getRenderX(false) + _imageWidth,
					y + getRenderY(false) + renderHeight - _vehicle->getHeight());

		if (!_title.empty()) {
			x += getRenderCenterX() - _textWidth / 2;
			y += getRenderY() + 10;

			_font->print(_title, _fontColor, x, y);
		}
	}

	int getY () const
	{
		if (!_title.empty())
			return getRenderY(false) + 10 + _textHeight;
		return getRenderY(false);
	}
};
