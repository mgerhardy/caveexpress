#pragma once

#include "ui/nodes/UINode.h"
#include "ui/BitmapFont.h"

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
	UINodeBackground (IFrontend *frontend, const std::string& title, bool showVehicle = true);

	virtual ~UINodeBackground ()
	{
	}

	inline void setAmountHorizontal (int amount)
	{
		setSize(1.0, 1.0f);
		setSize((float)amount * (float)_imageWidth / static_cast<float>(getRenderWidth()), getHeight());
		_amountHorizontal = amount;
	}

	inline void setAmountVertical (int amount)
	{
		setSize(1.0, 1.0f);
		setSize(getWidth(), (float)amount * (float)_imageHeight / static_cast<float>(getRenderHeight()));
		_amountVertical = amount;
	}

	inline void setAmount (int amountHorizontal, int amountVertical)
	{
		setSize(1.0, 1.0f);
		setSize((float)amountHorizontal * (float)_imageWidth / static_cast<float>(getRenderWidth()), (float)amountVertical * (float)_imageHeight / static_cast<float>(getRenderHeight()));
		_amountHorizontal = amountHorizontal;
		_amountVertical = amountVertical;
	}

	virtual TexturePtr getCave () const
	{
		return _cave;
	}

	virtual TexturePtr getCaveArt () const
	{
		return _caveArt;
	}

	void renderMiddle (int x, int y) const override;
	void onWindowResize () override;

	int getY () const
	{
		if (!_title.empty())
			return getRenderY(false) + 10 + _textHeight;
		return getRenderY(false);
	}
};
