#include "UIFillLayout.h"
#include "ui/nodes/UINode.h"

UIFillLayout::UIFillLayout (bool horizontal) :
		IUILayout(), _horizontal(horizontal)
{
}

UIFillLayout::~UIFillLayout ()
{
}

void UIFillLayout::layout (UINode* parent)
{
	float maxSize = 0.0f;
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		const float size = _horizontal ? node->getWidth() : node->getHeight();
		maxSize = std::max(size, maxSize);
	}

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		const float size = _horizontal ? node->getWidth() : node->getHeight();
		maxSize = std::max(size, maxSize);
	}

	float currentPos = 0.0f;
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		if (_horizontal) {
			node->setPos(currentPos, node->getY());
		} else {
			node->setPos(node->getX(), currentPos);
		}
		currentPos += maxSize;
	}
}
