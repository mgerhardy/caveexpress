#include "UINodeMapEditorSelectedItem.h"
#include "engine/client/ui/UI.h"
#include "engine/client/sprites/Sprite.h"
#include "engine/client/ui/BitmapFont.h"

UINodeMapEditorSelectedItem::UINodeMapEditorSelectedItem (IFrontend *frontend) :
		UINode(frontend, "selected-item-node")
{
	setBorder(true);
	setBorderColor(colorWhite);
}

UINodeMapEditorSelectedItem::~UINodeMapEditorSelectedItem ()
{
}

void UINodeMapEditorSelectedItem::render (int x, int y) const
{
	UINode::render(x, y);

	if (!_activeSpriteDefition)
		return;

	const BitmapFontPtr& font = getFont();
	const int textOffset = 10;
	const SpritePtr sprite = UI::get().loadSprite(_activeSpriteDefition->id);
	const int nodeX = x + getRenderX();
	const int nodeY = y + getRenderY();
	const int nodeW = getRenderWidth();
	const int nodeH = getRenderHeight() - textOffset - font->getCharHeight();
	const int spriteW = sprite->getMaxWidth();
	const int spriteH = sprite->getMaxHeight();
	const float aspectW = spriteW / static_cast<float>(nodeW);
	const float aspectH = spriteH / static_cast<float>(nodeH);
	float widthF;
	float heightF;
	if (aspectW > aspectH) {
		widthF = spriteW / aspectW;
		heightF = spriteH / aspectW;
	} else {
		widthF = spriteW / aspectH;
		heightF = spriteH / aspectH;
	}

	const int _x = getRenderCenterX() - widthF / 2;
	const int _y = nodeY;
	const int w = widthF;
	const int h = heightF;

	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		sprite->render(_frontend, layer, _x, _y, w, h);
	}

	std::string name = sprite->getName();
	int textWidth = font->getTextWidth(name);
	while (textWidth > nodeW) {
		name = name.substr(0, name.length() - 1);
		if (name.empty())
			return;
		textWidth = font->getTextWidth(name);
	}

	const int fontX = nodeX + nodeW / 2 - textWidth / 2;
	const int fontY = _y + h + textOffset;
	font->print(name, colorWhite, fontX, fontY);
}
