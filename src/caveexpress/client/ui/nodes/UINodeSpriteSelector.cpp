#include "UINodeSpriteSelector.h"
#include "engine/client/ui/UI.h"
#include "engine/common/SpriteType.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"

UINodeSpriteSelector::UINodeSpriteSelector (IFrontend *frontend, int cols, int rows) :
		UINodeSelector<SpriteDefPtr>(frontend, cols, rows, 40 / static_cast<float>(frontend->getWidth()), 40 / static_cast<float>(frontend->getHeight())), _theme(&ThemeType::NONE)
{
	setBorder(true);
}

UINodeSpriteSelector::~UINodeSpriteSelector ()
{
}

void UINodeSpriteSelector::renderSelectorEntry (int index, const SpriteDefPtr& data, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	const SpritePtr& sprite = UI::get().loadSprite(data->id);
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		sprite->render(_frontend, layer, x, y, colWidth, rowHeight, data->angle, alpha);
	}
	if (_selection && data->id == (*_selection)->id) {
		_frontend->renderRect(x, y, colWidth, rowHeight, colorYellow);
	}
}

void UINodeSpriteSelector::update (uint32_t deltaTime)
{
	UINodeSelector<SpriteDefPtr>::update(deltaTime);
	for (SelectorEntryIter i = _entries.begin(); i != _entries.end(); ++i) {
		const SpritePtr& sprite = UI::get().loadSprite((*i)->id);
		sprite->update(deltaTime);
	}
}

void UINodeSpriteSelector::addSprites (const ThemeType& theme)
{
	_theme = &theme;
	reset();
	for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
		const SpriteDefPtr& sprite = i->second;
		if (!sprite->theme.isNone() && sprite->theme != theme)
			continue;
		const SpriteType& type = sprite->type;
		if (!SpriteTypes::isMapTile(type) && !SpriteTypes::isLiane(type))
			continue;
		if (SpriteTypes::isGeyser(type) && sprite->isStatic())
			continue;
		if (SpriteTypes::isPackageTarget(type) && !sprite->isStatic())
			continue;
		addData(sprite);
	}

	if (_entries.empty()) {
		_selection = nullptr;
		_selectedIndex = -1;
		return;
	}

	_selectedIndex = 0;
	select();
}
