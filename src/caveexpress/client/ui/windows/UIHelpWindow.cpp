#include "UIHelpWindow.h"
#include "client/ui/nodes/UINodeSprite.h"
#include "client/ui/nodes/UINodeKey.h"
#include "client/ui/nodes/UINodeLabel.h"
#include "client/ui/nodes/UINodeSprite.h"
#include "client/ui/UI.h"
#include "common/SpriteDefinition.h"

UIHelpWindow::UIHelpWindow (const std::string& name, IFrontend* frontend, int flags) :
		UIWindow(name, frontend, flags), _currentX(0.01f), _currentY(0.0f), _internalPadding(0.001f), _iconSize(0.08f), _iconGap(
				0.01f)
{
}

UIHelpWindow::~UIHelpWindow ()
{
}

void UIHelpWindow::addKeys (const std::vector<std::string>& keys)
{
	if (keys.empty())
		return;
	const UINode *node = _nodes.empty() ? nullptr : _nodes.back();
	const float pos = (node != nullptr ? node->getRight() : 0.0f) + _iconGap;

	UINodeKey *keyNode = new UINodeKey(_frontend, keys.front(), pos, _currentY + _iconSize / 4.0, _iconSize / 2.0);
	add(keyNode);
}

void UIHelpWindow::addKey (const std::string& key)
{
	const UINode *node = _nodes.empty() ? nullptr : _nodes.back();
	const float pos = (node != nullptr ? node->getRight() : 0.0f) + _iconGap;

	UINodeKey *keyNode = new UINodeKey(_frontend, key, pos, _currentY + _iconSize / 4.0, _iconSize / 2.0);
	add(keyNode);
}

UINodeLabel* UIHelpWindow::addString (const std::string& string, const std::string& font)
{
	const UINode *node = _nodes.empty() ? nullptr : _nodes.back();
	const float pos = (node != nullptr ? node->getRight() : 0.0f) + _iconGap;

	UINodeLabel *labelNode = new UINodeLabel(_frontend, string, getFont(font));
	labelNode->setPos(pos, _currentY + node->getHeight() / 2.0f - labelNode->getHeight() / 2.0f);
	add(labelNode);
	return labelNode;
}

UINodeSprite* UIHelpWindow::addSprite (const EntityType& type, const Animation& animation)
{
	return addSprite(type, animation, _iconSize, _iconSize);
}

UINodeSprite* UIHelpWindow::addSprite (const EntityType& type, const Animation& animation, float w, float h)
{
	const UINode *node = _nodes.empty() ? nullptr : _nodes.back();
	const float pos = (node != nullptr ? node->getRight() : 0.0f) + _iconGap;
	UINodeSprite *spriteNode = addSpriteNode(type, animation, pos, _currentY, w, h);
	spriteNode->setAspectRatioSize(w, h, 1.3f);
	spriteNode->alignToMiddle();
	return spriteNode;
}

UINode *UIHelpWindow::addTexture (const std::string& texture)
{
	const UINode *node = _nodes.empty() ? nullptr : _nodes.back();
	const float pos = (node != nullptr ? node->getRight() : 0.0f) + _iconGap;
	return addTextureNode(texture, pos, _currentY, _iconSize, _iconSize);
}

UINodeSprite* UIHelpWindow::addSpriteNode (const EntityType& type, const Animation& animation, float x, float y, float w, float h)
{
	const std::string spriteName = SpriteDefinition::get().getSpriteName(type, animation);
	const SpritePtr& spritePtr = UI::get().loadSprite(spriteName);
	UINodeSprite* spriteNode = new UINodeSprite(_frontend);
	spriteNode->addSprite(spritePtr);
	spriteNode->setPos(x, y);
	spriteNode->setSize(w, h);
	add(spriteNode);
	return spriteNode;
}
