#pragma once

#include "common/EventHandler.h"
#include "common/System.h"
#include "common/IFrontend.h"
#include "textures/Texture.h"
#include <string>
#include <deque>

// forward decl
class UINode;
class IUILayout;

#define tr(in) UI::get().translate(in)

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
typedef std::shared_ptr<UINodeListener> UINodeListenerPtr;

class OpenURLListener: public UINodeListener {
protected:
	const std::string _url;
	IFrontend *_frontend;
	bool _newWindow;
public:
	OpenURLListener (IFrontend *frontend, const std::string& url, bool newWindow = true) :
			_url(url), _frontend(frontend), _newWindow(newWindow)
	{
	}

	void onClick ()
	{
		_frontend->minimize();
		System.openURL(_url, _newWindow);
	}
};

#ifdef __EMSCRIPTEN__
	class EmscriptenFullscreenListener: public UINodeListener {
	public:
		void onClick () {
			EM_ASM({
				Module.requestFullScreen();
			});
		}
	};
#endif

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
	friend class UIWindow;
protected:
	static int _counter;

	struct NodeCoord {
		float x;
		float y;

		NodeCoord (float _x, float _y) :
				x(_x), y(_y)
		{
		}

		NodeCoord () :
				x(0.0f), y(0.0f)
		{
		}
	};
	NodeCoord _pos;
	NodeCoord _size;
	NodeCoord _minSize;
	float _padding;
	float _marginTop;
	float _marginLeft;

	typedef std::vector<UINode*> UINodeList;
	typedef UINodeList::iterator UINodeListIter;
	typedef UINodeList::const_iterator UINodeListConstIter;
	typedef UINodeList::reverse_iterator UINodeListRevIter;
	typedef UINodeList::const_reverse_iterator UINodeListConstRevIter;
	UINodeList _nodes;

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
	uint32_t _flashMillis;
	float _originalAlpha;
	bool _autoId;

	struct UINodeDelayedText {
		std::string text;
		uint32_t delayMillis;
		NodeCoord pos;
		BitmapFontPtr font;

		UINodeDelayedText (const std::string& _text, uint32_t _delayMillis, const NodeCoord& _textPos, const BitmapFontPtr& _font) :
				text(_text), delayMillis(_delayMillis), pos(_textPos), font(_font)
		{
		}
	};

	typedef std::deque<UINodeDelayedText> DelayedTexts;
	typedef DelayedTexts::iterator DelayedTextsIter;
	typedef DelayedTexts::const_iterator DelayedTextsConstIter;
	DelayedTexts _texts;
	IUILayout* _layout;
	UINode* _parent;
	std::string _tooltip;
	bool _fingerPressed;
	bool _mousePressed;

	TexturePtr loadTexture (const std::string& name) const;
	void renderImage (const TexturePtr& texture, int x, int y, int w = -1, int h = -1, float alpha = 1.0f) const;
	void renderImage (const std::string& texture, int x, int y, int w, int h, float alpha = 1.0f) const;
	void renderRect (int x, int y, int w, int h, const Color& color) const;
	void renderFilledRect (int x, int y, int w, int h, const Color& rgba) const;
	void renderLine (int x1, int y1, int x2, int y2, const Color& rgba = colorNull) const;

	virtual void renderDebug (int x, int y, int textY) const;

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
			return _texture->getWidth() / static_cast<float>(_frontend->getWidth()) + 2.0f * getPadding();
		return 0.0f;
	}

	virtual float getAutoHeight () const
	{
		if (_texture)
			return _texture->getHeight() / static_cast<float>(_frontend->getHeight()) + 2.0f * getPadding();
		return 0.0f;
	}

	inline void autoSize ()
	{
		const float w = std::max(_minSize.x, getAutoWidth());
		const float h = std::max(_minSize.y, getAutoHeight());
		setSize(w, h);
	}

	void updateAlignment ();

	inline float getParentHeightf () const;
	inline float getParentWidthf () const;
	inline float getParentXf () const;
	inline float getParentYf () const;

	BitmapFontPtr getFont (const std::string& font = "") const;
	bool isSmallScreen() const;

	virtual bool execute ();

public:
	UINode (IFrontend *frontend, const std::string& id = "");
	virtual ~UINode ();

	void doLayout ();
	void setId (const std::string& id);
	const std::string& getId () const;

	// checks whether the given coordinates are inside the node area
	virtual bool checkBounds (int x, int y) const;
	void setFocusAlpha (float focusAlpha);

	void addFront (UINode* node);
	// inserts a node before another node - can be useful for the focus order
	void addBefore (UINode* reference, UINode* node);
	virtual void add (UINode* node);

	bool hasImage () const;
	virtual const TexturePtr& setImage (const std::string& texture);
	// return false if the window that contains this node should not get removed from the ui stack. true otherwise
	virtual bool onPop ();
	// return false if the window that contains this node should not get pushed. true otherwise
	virtual bool onPush ();
	virtual void addFocus (int32_t x, int32_t y);
	virtual void removeFocus ();
	virtual void update (uint32_t deltaTime);
	virtual void render (int x, int y) const;
	virtual void renderBack (int x, int y) const;
	virtual void renderMiddle (int x, int y) const;
	virtual void renderTop (int x, int y) const;
	virtual void renderOnTop (int x, int y) const;
	virtual void initDrag (int32_t x, int32_t y);
	virtual void handleDrop (uint16_t x, uint16_t y);

	virtual void setPos (float x = 0.0f, float y = 0.0f);
	virtual void setSize (float w, float h = 1.0f);
	virtual void setPadding (float padding);
	// this command is executed whenever the node is activated.
	// This is e.g. a left mouse release or a finger release.
	virtual void setOnActivate (const std::string& onActivateCmd);
	virtual void setAlpha (float alpha);
	float getAlpha () const;
	void restoreAlpha ();
	void setMinWidth (float minWidth);
	void setMinHeight (float minHeight);
	float getPadding () const;
	void setTooltip (const std::string& tooltip);
	void setLayout (IUILayout* layout);

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
	virtual bool runFocusNode ();
	bool checkFocus (int32_t x, int32_t y);
	virtual bool isActive () const;
	UINode* getNode (const std::string& nodeId);
	virtual bool nextFocus ();
	virtual bool prevFocus ();
	virtual bool addLastFocus ();
	virtual bool addFirstFocus ();

	inline IFrontend* getFrontend () { return _frontend; }

	inline int getRenderX (bool includePadding = true) const
	{
		return getRenderXf(includePadding) * _frontend->getWidth();
	}

	inline int getRenderY (bool includePadding = true) const
	{
		return getRenderYf(includePadding) * _frontend->getHeight();
	}

	inline int getRenderCenterX () const
	{
		return getRenderCenterXf() * _frontend->getWidth();
	}

	inline int getRenderCenterY () const
	{
		return getRenderCenterYf() * _frontend->getHeight();
	}

	inline int getRenderWidth (bool includePadding = true) const
	{
		return getRenderWidthf(includePadding) * _frontend->getWidth();
	}

	inline int getRenderHeight (bool includePadding = true) const
	{
		return getRenderHeightf(includePadding) * _frontend->getHeight();
	}

	inline float getRenderXf (bool includePadding = true) const
	{
		if (includePadding)
			return _pos.x + _padding + _marginLeft;
		return _pos.x + _marginLeft;
	}

	inline float getRenderYf (bool includePadding = true) const
	{
		if (includePadding)
			return _pos.y + _padding + _marginTop;
		return _pos.y + _marginTop;
	}

	inline float getRenderCenterXf () const
	{
		return getRenderWidthf() / 2.0f + getRenderXf();
	}

	inline float getRenderCenterYf () const
	{
		return getRenderHeightf() / 2.0f + getRenderYf();
	}

	inline float getRenderWidthf (bool includePadding = true) const
	{
		if (includePadding)
			return _size.x - _padding * 2.0f;
		return _size.x;
	}

	inline float getRenderHeightf (bool includePadding = true) const
	{
		if (includePadding)
			return _size.y - _padding * 2.0f;
		return _size.y;
	}

	void alignTo (const UINode* node, int align = NODE_ALIGN_CENTER | NODE_ALIGN_MIDDLE, float padding = 0.0f);
	void flash (uint32_t flashMillis = 2000);

	virtual void displayText (const std::string& text, uint32_t delayMillis = 3000, float x = -1.0f, float y = -1.0f);

	virtual bool onTextInput (const std::string& text);
	virtual bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y);
	virtual bool onFingerPress (int64_t finger, uint16_t x, uint16_t y);
	virtual bool onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy);
	virtual bool onKeyPress (int32_t key, int16_t modifier);
	virtual bool onKeyRelease (int32_t key);
	virtual bool onMouseButtonRelease (int32_t x, int32_t y, unsigned char button);
	virtual bool onMouseButtonPress (int32_t x, int32_t y, unsigned char button);
	virtual bool onGesture (int64_t gestureId, float error, int32_t numFingers);
	/**
	 * @param[in] theta the amount that the fingers rotated during this motion
	 * @param[in] dist the amount that the fingers pinched during this motion
	 * @param[in] numFingers the number of fingers used in the gesture
	 */
	virtual bool onMultiGesture (float theta, float dist, int32_t numFingers);
	virtual bool onGestureRecord (int64_t gestureId);
	virtual bool onMouseLeftRelease (int32_t x, int32_t y);
	virtual bool onMouseMiddleRelease (int32_t x, int32_t y);
	virtual bool onMouseRightRelease (int32_t x, int32_t y);
	virtual bool onMouseLeftPress (int32_t x, int32_t y);
	virtual bool onMouseMiddlePress (int32_t x, int32_t y);
	virtual bool onMouseRightPress (int32_t x, int32_t y);
	virtual bool onMouseWheel (int32_t x, int32_t y);
	virtual void onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY);
	/**
	 * @param[in] value The relative value that the joystick was moved, [-32768,32767]
	 */
	virtual bool onJoystickMotion (bool horizontal, int value);
	virtual bool onJoystickButtonPress (int x, int y, uint8_t button);
	virtual bool onControllerButtonPress (int x, int y, const std::string& button);

	virtual void onAdd ();

	void setParent (UINode *parent);
	UINode *getParent ();

	void centerUnder (UINode *node, float gap = 0.01f);
	void putUnder (UINode *node, float gap = 0.01f);
	void putUnderRight (UINode *node, float gap = 0.01f);
	void putAbove (UINode *node, float gap = 0.01f);
	void putAboveRight (UINode *node, float gap = 0.01f);
	void putRight (UINode *node, float gap = 0.01f);
	void putLeft (UINode *node, float gap = 0.01f);

	bool isVisible () const;
	virtual void setVisible (bool visible);
	void setAlignment (int align);
	void setMargin (float top, float left);
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

	inline float getScreenPadding ()
	{
		return System.getScreenPadding() / static_cast<float>(std::max(_frontend->getWidth(), _frontend->getHeight()));
	}

	inline void setSystemPadding ()
	{
		setPadding(getScreenPadding());
	}

	inline bool hasFocus () const
	{
		return _focus;
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

inline void UINode::centerUnder (UINode *node, float gap)
{
	setPos(node->getCenter() - getWidth() / 2.0f, node->getBottom() + gap);
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

inline void UINode::setOnActivate (const std::string& onActivateCmd)
{
	_onActivate = onActivateCmd;
}

inline void UINode::setId (const std::string& id)
{
	_id = id;
	_autoId = false;
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

inline void UINode::setMargin (float top, float left)
{
	_marginTop = top;
	_marginLeft = left;
}

inline void UINode::setParent (UINode *parent)
{
	_parent = parent;
}

inline UINode *UINode::getParent ()
{
	return _parent;
}
