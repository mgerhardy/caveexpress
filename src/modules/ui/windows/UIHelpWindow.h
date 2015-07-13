#pragma once
#include "ui/windows/UIWindow.h"
#include "common/Animation.h"
#include "common/EntityType.h"
#include "common/NonCopyable.h"

class UINodeSprite;
class UINodeLabel;

class UIHelpWindow: public UIWindow {
protected:
	class AutoBorder : public NonCopyable {
	protected:
		UIHelpWindow* _window;
		UINode *_border;
	public:
		AutoBorder (UIHelpWindow* window, float y) :
				_window(window)
		{
			_border = new UINode(_window->_frontend);
			_border->setBorder(true);
			_border->setBorderColor(colorWhite);
			_border->setBackgroundColor(backgroundColor);
			_border->setStandardPadding();
			_border->setPos(_window->_currentX - getPadding(), y - getPadding());
			_window->add(_border);
		}

		inline void setBackgroundColor(const Color& color)
		{
			_border->setBackgroundColor(color);
		}

		inline void setBorderColor(const Color& color)
		{
			_border->setBorderColor(color);
		}

		inline float getPadding () const
		{
			return _window->_internalPadding - _border->getPadding();
		}

		inline float getWidth () const
		{
			const UINode *node = _window->_nodes.back();
			const float w = node->getRight() + 2.0f * _window->_internalPadding - _window->_currentX
					+ _window->_iconGap;
			return w;
		}

		inline float getHeight () const
		{
			const float h = _window->_iconSize + 2.0f * _window->_internalPadding;
			return h;
		}

		virtual ~AutoBorder ()
		{
			const float w = getWidth();
			const float h = getHeight();
			_border->setSize(w + _border->getPadding() * 2.0f, h + _border->getPadding() * 2.0f);
		}
	};

	float _currentX;
	float _currentY;
	float _internalPadding;
	float _iconSize;
	float _iconGap;

	UINodeLabel* addString (const std::string& string, const std::string& font = "");
	void addKeys (const std::vector<std::string>& keys);
	void addKey (const std::string& key);
	UINodeSprite* addSprite (const EntityType& type, const Animation& animation, float w, float h);
	UINode* addTexture (const std::string& texture);
	UINodeSprite* addSpriteNode (const EntityType& type, const Animation& animation, float x, float y, float w, float h);
public:
	UIHelpWindow (const std::string& name, IFrontend* frontend, int flags);
	virtual ~UIHelpWindow ();
};
