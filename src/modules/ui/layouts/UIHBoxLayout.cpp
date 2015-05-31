#include "UIHBoxLayout.h"
#include "ui/nodes/UINode.h"

UIHBoxLayout::UIHBoxLayout (float spacing, bool expandChildren, int align) :
		IUILayout(), _spacing(spacing), _expandChildren(expandChildren), _align(align)
{
}

UIHBoxLayout::~UIHBoxLayout ()
{
}

void UIHBoxLayout::layout (UINode* parent)
{
	float currentSize = 2.0f * parent->getPadding();
	float minHeight = 0.0f;

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		currentSize += node->getWidth() + _spacing;
		minHeight = std::max(node->getHeight() + 2.0f * parent->getPadding(), minHeight);
	}

	float size = currentSize - _spacing;
	parent->setSize(size, minHeight);

	// the size might change the alignment and thus the position of the parent
	float currentPos = parent->getPadding();
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		if (_expandChildren)
			node->setSize(node->getWidth(), parent->getHeight() - 2.0f * parent->getPadding());

		if (_align & NODE_ALIGN_MIDDLE) {
			node->setPos(currentPos, parent->getHeight() / 2.0f - node->getHeight() / 2.0f);
		} else if (_align & NODE_ALIGN_BOTTOM) {
			node->setPos(currentPos, parent->getHeight() - node->getHeight());
		} else {
			node->setPos(currentPos, node->getY());
		}

		currentPos += node->getWidth() + _spacing;
	}
}
