#pragma once

#include "shared/EventHandler.h"
#include "common/System.h"
#include "shared/IFrontend.h"
#include "client/textures/Texture.h"
#include <string>

// forward decl
class UINode;

class UINodeListener {
public:
	virtual ~UINodeListener ()
	{
	}
	virtual void onClick ()
	{
	}
	virtual void onAddFocus ()
	{
	}
	virtual void onRemoveFocus ()
	{
	}
	virtual void onValueChanged ()
	{
	}
};
typedef SharedPtr<UINodeListener> UINodeListenerPtr;

class OpenURLListener: public UINodeListener {
protected:
	const std::string _url;
	IFrontend *_frontend;
public:
	OpenURLListener (IFrontend *frontend, const std::string& url) :
			_url(url), _frontend(frontend)
	{
	}

	void onClick ()
	{
		_frontend->minimize();
		System.openURL(_url);
	}
};

enum UINodeAlign {
	// horizontal
	NODE_ALIGN_LEFT = 1 << 0,
	NODE_ALIGN_RIGHT = 1 << 1,
	NODE_ALIGN_CENTER = 1 << 2,

	// vertical
	NODE_ALIGN_TOP = 1 << 3,
	NODE_ALIGN_BOTTOM = 1 << 4,
	NODE_ALIGN_MIDDLE = 1 << 5
};

class UINode {
protected:
	static int _counter;

	struct NodeCoord {
		float x;
		float y;

		NodeCoord () :
				x(0.0f), y(0.0f)
		{
		}
	};
	NodeCoord _pos;
	NodeCoord _size;
	NodeCoord _minSize;
	float _padding;
	float _spacingX;
	float _spacingY;

protected:
	friend class UINodePanel;
	TexturePtr _texture;
	std::string _onActivate;
	bool _focus;
	float _focusAlpha;
	// the focus mouse x coordinate
	int _focusMouseX;
	// the focus mouse y coordinate
	int _focusMouseY;
	bool _visible;
	bool _enabled;
	// start positions of a drag
	int _dragStartX;
	int _dragStartY;
	typedef std::vector<UINodeListenerPtr> Listeners;
	Listeners _listeners;
	float _alpha;
	float _previousAlpha;
	std::string _id;
	bool _renderBorder;
	Color _borderColor;
	Color _backgroundColor;
	IFrontend *_frontend;
	int _align;
	uint32_t _time;

	class UINodeDelayedText {
	public:
		std::string text;
		uint32_t delayMillis;
		NodeCoord pos;

		UINodeDelayedText () :
				text(""), delayMillis(0)
		{
		}

		void set (const std::string& __text, uint32_t __delayMillis, float __x, float __y)
		{
			text = __text;
			delayMillis = __delayMillis;
			pos.x = __x;
			pos.y = __y;
		}
	};

	UINodeDelayedText _text;

	TexturePtr loadTexture (const std::string& name) const;
	void renderImage (const TexturePtr& texture, int x, int y, int w = -1, int h = -1, float alpha = 1.0f) const;
	void renderImage (const std::string& texture, int x, int y, int w, int h, float alpha = 1.0f) const;
	void renderRect (int x, int y, int w, int h, const Color& color) const;
	void renderFilledRect (int x, int y, int w, int h, const Color& rgba) const;
	void renderLine (int x1, int y1, int x2, int y2, const Color& rgba = colorNull) const;

	inline bool checkAABB (float x, float y, float aabbX, float aabbY, float aabbW, float aabbH) const
	{
		if (x < aabbX)
			return false;
		if (y < aabbY)
			return false;
		if (aabbW > 0.0f && x >= aabbX + aabbW)
			return false;
		if (aabbH > 0.0f && y >= aabbY + aabbH)
			return false;
		return true;
	}

	inline void enableScissor (int x, int y) const
	{
		enableScissor(x + getRenderX(), y + getRenderY(), getRenderWidth(), getRenderHeight());
	}

	inline void enableScissor (int x, int y, int w, int h) const
	{
		_frontend->enableScissor(x, y, w, h);
	}

	inline void disableScissor () const
	{
		_frontend->disableScissor();
	}

	virtual float getAutoWidth () const
	{
		if (_texture)
			return _texture->getWidth() / static_cast<float>(_frontend->getWidth());
		return 0.0f;
	}

	virtual float getAutoHeight () const
	{
		if (_texture)
			return _texture->getHeight() / static_cast<float>(_frontend->getHeight());
		return 0.0f;
	}

	inline void autoSize ()
	{
		const float w = std::max(_minSize.x, getAutoWidth());
		const float h = std::max(_minSize.y, getAutoHeight());
		setSize(w, h);
	}

	void updateAlignment ();

	BitmapFontPtr getFont (const std::string& font = "") const;

	void showAds ();
	void hideAds ();

public:
	UINode (IFrontend *frontend, const std::string& id = "");
	virtual ~UINode ();

	void setId (const std::string& id);
	const std::string& getId () const;

	// checks whether the given coordinates are inside the node area
	virtual bool checkBounds (int x, int y) const;
	virtual bool wantFocus () const;
	void setFocusAlpha (float focusAlpha);

	bool hasImage () const;
	virtual const TexturePtr& setImage (const std::string& texture);
	// return false if the window that contains this node should not get removed from the ui stack. true otherwise
	virtual bool onPop ();
	// return false if the window that contains this node should not get pushed. true otherwise
	virtual bool onPush ();
	virtual void addFocus (int32_t x = -1, int32_t y = -1);
	virtual void removeFocus ();
	virtual void update (uint32_t deltaTime);
	virtual void render (int x, int y) const;
	virtual void initDrag (int32_t x, int32_t y);
	virtual void handleDrop (uint16_t x, uint16_t y);
	virtual bool execute (int x = -1, int y = -1);
	virtual bool hasMultipleFocus ();

	virtual void setPos (float x = 0.0f, float y = 0.0f);
	virtual void setSize (float w, float h = 1.0f);
	virtual void setPadding (float padding);
	virtual void setSpacing (float spacingX, float spacingY);
	// this command is executed whenever the node is activated.
	// This is e.g. a left mouse release or a finger release.
	virtual void setOnActivate (const std::string& onActivateCmd);
	virtual void setAlpha (float alpha);
	float getAlpha () const;
	void restoreAlpha ();
	void setMinWidth (float minWidth);
	void setMinHeight (float minHeight);
	float getPadding () const;
	float getSpacingX () const;
	float getSpacingY () const;
	float getSpacingIntervalX (int interval) const;
	float getSpacingIntervalY (int interval) const;

	float getWidth () const;
	float getHeight () const;
	float getBottom () const;
	float getCenter () const;
	float getMiddle () const;
	float getTop () const;
	float getLeft () const;
	float getRight () const;
	float getX () const;
	float getY () const;

	inline int getRenderX (bool includePadding = true) const
	{
		if (includePadding)
			return (_pos.x + _padding) * _frontend->getWidth();
		return _pos.x * _frontend->getWidth();
	}

	inline int getRenderY (bool includePadding = true) const
	{
		if (includePadding)
			return (_pos.y + _padding) * _frontend->getHeight();
		return _pos.y * _frontend->getHeight();
	}

	inline int getRenderCenterX () const
	{
		return getRenderWidth() / 2 + getRenderX();
	}

	inline int getRenderCenterY () const
	{
		return getRenderHeight() / 2 + getRenderY();
	}

	inline int getRenderWidth (bool includePadding = true) const
	{
		if (includePadding)
			return (_size.x - _padding * 2.0f) * _frontend->getWidth();
		return _size.x * _frontend->getWidth();
	}

	inline int getRenderHeight (bool includePadding = true) const
	{
		if (includePadding)
			return (_size.y - _padding * 2.0f) * _frontend->getHeight();
		return _size.y * _frontend->getHeight();
	}

	void alignTo (const UINode* node, int align = NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE, float padding = 0.0f);

	virtual void displayText (const std::string& text, uint32_t delayMillis = 3000, float x = -1.0f, float y = -1.0f);

	virtual UINode* getNode (const std::string& nodeId);

	virtual bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y);
	virtual bool onFingerPress (int64_t finger, uint16_t x, uint16_t y);
	virtual void onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy);
	virtual bool onKeyPress (int32_t key, int16_t modifier);
	virtual bool onKeyRelease (int32_t key);
	virtual bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button);
	virtual bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button);

	virtual bool onMouseLeftRelease (int32_t x, int32_t y);
	virtual bool onMouseMiddleRelease (int32_t x, int32_t y);
	virtual bool onMouseRightRelease (int32_t x, int32_t y);
	virtual bool onMouseLeftPress (int32_t x, int32_t y);
	virtual bool onMouseMiddlePress (int32_t x, int32_t y);
	virtual bool onMouseRightPress (int32_t x, int32_t y);
	virtual bool onMouseWheel (int32_t x, int32_t y);
	virtual bool onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY);
	virtual bool onJoystickMotion (bool horizontal, int value);
	virtual bool onJoystickButtonPress (int x, int y, uint8_t button);
	virtual bool onControllerButtonPress (int x, int y, const std::string& button);

	void putUnder (UINode *node, float gap = 0.01f);
	void putUnderRight (UINode *node, float gap = 0.01f);
	void putAbove (UINode *node, float gap = 0.01f);
	void putAboveRight (UINode *node, float gap = 0.01f);
	void putRight (UINode *node, float gap = 0.01f);
	void putLeft (UINode *node, float gap = 0.01f);

	bool isVisible () const;
	void setVisible (bool visible);
	void setAlignment (int align);
	void setEnabled (bool enable);

	void setBorder (bool border);
	void setBackgroundColor (const Color color);
	void setBorderColor (const Color color);

	inline void addListener (const UINodeListenerPtr &listener)
	{
		_listeners.push_back(listener);
	}

	inline void removeListener (const UINodeListenerPtr &listener)
	{
		Listeners::iterator i = std::remove(_listeners.begin(), _listeners.end(), listener);
		if (i == _listeners.end())
			return;
		_listeners.erase(i, _listeners.end());
	}

	inline void setStandardPadding ()
	{
		setPadding(2.0f / static_cast<float>(_frontend->getWidth()));
	}

	inline void setStandardSpacing ()
	{
		setSpacing(0.03125f, 0.03125f * (_frontend->getWidth() / static_cast<float>(_frontend->getHeight())));
	}

	inline bool hasFocus () const
	{
		return _focus;
	}

	inline void setRefSize (int width, int height)
	{
		setSize(width / 1024.0f, height / 768.0f);
	}
};

inline void UINode::putAboveRight (UINode *node, float gap)
{
	setPos(node->getRight() - getWidth(), node->getTop() - gap - getHeight());
}

inline void UINode::putUnderRight (UINode *node, float gap)
{
	setPos(node->getRight() - getWidth(), node->getBottom() + gap);
}

inline void UINode::putAbove (UINode *node, float gap)
{
	setPos(node->getLeft(), node->getTop() - gap - getHeight());
}

inline void UINode::putUnder (UINode *node, float gap)
{
	setPos(node->getLeft(), node->getBottom() + gap);
}

inline void UINode::putRight (UINode *node, float gap)
{
	setPos(node->getRight() + gap, node->getTop());
}

inline void UINode::putLeft (UINode *node, float gap)
{
	setPos(node->getLeft() - gap - getWidth(), node->getTop());
}

inline void UINode::setVisible (bool visible)
{
	_visible = visible;
}

inline void UINode::setEnabled (bool enable)
{
	_enabled = enable;
}

inline bool UINode::isVisible () const
{
	return _visible;
}

inline void UINode::setBackgroundColor (const Color color)
{
	for (int i = 0; i < 4; ++i) {
		_backgroundColor[i] = color[i];
	}
}

inline void UINode::setBorderColor (const Color color)
{
	for (int i = 0; i < 4; ++i) {
		_borderColor[i] = color[i];
	}
}

inline void UINode::setBorder (bool border)
{
	_renderBorder = border;
}

inline float UINode::getX () const
{
	return _pos.x;
}

inline float UINode::getY () const
{
	return _pos.y;
}

inline float UINode::getWidth () const
{
	return _size.x;
}

inline float UINode::getHeight () const
{
	return _size.y;
}

inline float UINode::getBottom () const
{
	return getTop() + getHeight();
}

inline float UINode::getCenter () const
{
	return getLeft() + getWidth() / 2.0f;
}

inline float UINode::getMiddle () const
{
	return getTop() + getHeight() / 2.0f;
}

inline float UINode::getTop () const
{
	return getY();
}

inline float UINode::getLeft () const
{
	return getX();
}

inline float UINode::getRight () const
{
	return getLeft() + getWidth();
}

inline float UINode::getPadding () const
{
	return _padding;
}

inline float UINode::getSpacingX () const
{
	return _spacingX;
}

inline float UINode::getSpacingY () const
{
	return _spacingY;
}

inline float UINode::getSpacingIntervalX (int interval) const
{
	return interval * _spacingX;
}

inline float UINode::getSpacingIntervalY (int interval) const
{
	return interval * _spacingY;
}

inline void UINode::setMinWidth (float minWidth)
{
	_minSize.x = minWidth;
}

inline void UINode::setMinHeight (float minHeight)
{
	_minSize.y = minHeight;
}

inline void UINode::setAlpha (float alpha)
{
	_previousAlpha = _alpha;
	_alpha = clamp(alpha, 0.0f, 1.0f);
}

inline void UINode::restoreAlpha ()
{
	_alpha = _previousAlpha;
}

inline void UINode::setPadding (float padding)
{
	_padding = padding;
}

inline void UINode::setSpacing (float spacingX, float spacingY)
{
	_spacingX = spacingX;
	_spacingY = spacingY;
}

inline void UINode::setOnActivate (const std::string& onActivateCmd)
{
	_onActivate = onActivateCmd;
}

inline void UINode::setId (const std::string& id)
{
	_id = id;
}

inline const std::string& UINode::getId () const
{
	return _id;
}

inline void UINode::setFocusAlpha (float focusAlpha)
{
	_focusAlpha = focusAlpha;
}

inline float UINode::getAlpha () const
{
	return _alpha;
}
