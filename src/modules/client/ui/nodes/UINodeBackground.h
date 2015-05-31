#pragma once

#include "client/ui/nodes/UINode.h"
#include "common/String.h"
#include "client/ui/BitmapFont.h"
#include "common/Payment.h"
#include "common/Config.h"

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
		setSize(amount * _imageWidth / static_cast<float>(getRenderWidth()), getHeight());
		_amountHorizontal = amount;
	}

	inline void setAmountVertical (int amount)
	{
		setSize(1.0, 1.0f);
		setSize(getWidth(), amount * _imageHeight / static_cast<float>(getRenderHeight()));
		_amountVertical = amount;
	}

	inline void setAmount (int amountHorizontal, int amountVertical)
	{
		setSize(1.0, 1.0f);
		setSize(amountHorizontal * _imageWidth / static_cast<float>(getRenderWidth()), amountVertical * _imageHeight / static_cast<float>(getRenderHeight()));
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

	virtual void renderMiddle (int x, int y) const override;

	int getY () const
	{
		if (!_title.empty())
			return getRenderY(false) + 10 + _textHeight;
		return getRenderY(false);
	}
};
