#include "UIVBoxLayout.h"
#include "ui/nodes/UINode.h"

UIVBoxLayout::UIVBoxLayout (float spacing, bool expandChildren, int align) :
		IUILayout(), _spacing(spacing), _expandChildren(expandChildren), _align(align)
{
}

UIVBoxLayout::~UIVBoxLayout ()
{
}

void UIVBoxLayout::layout (UINode* parent)
{
	float currentSize = 2.0f * parent->getPadding();
	float minWidth = 0.0f;

	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		currentSize += node->getHeight() + _spacing;
		minWidth = std::max(node->getWidth() + 2.0f * parent->getPadding(), minWidth);
	}

	float size = currentSize - _spacing;
	parent->setSize(minWidth, size);

	// the size might change the alignment and thus the position of the parent
	float currentPos = parent->getPadding();
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		UINode* node = *i;
		if (_expandChildren)
			node->setSize(parent->getWidth() - 2.0f * parent->getPadding(), node->getHeight());

		if (_align & NODE_ALIGN_CENTER) {
			node->setPos(parent->getWidth() / 2.0f - node->getWidth() / 2.0f, currentPos);
		} else if (_align & NODE_ALIGN_LEFT) {
			node->setPos(0.0f, currentPos);
		} else if (_align & NODE_ALIGN_RIGHT) {
			node->setPos(parent->getWidth() - node->getWidth(), currentPos);
		} else {
			node->setPos(node->getX(), currentPos);
		}
		currentPos += node->getHeight() + _spacing;
	}
}
