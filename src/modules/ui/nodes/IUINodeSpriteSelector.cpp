#include <ui/nodes/IUINodeSpriteSelector.h>
#include "ui/UI.h"
#include "common/SpriteType.h"

IUINodeSpriteSelector::IUINodeSpriteSelector (IFrontend *frontend, int cols, int rows) :
		Super(frontend, cols, rows, 40 / static_cast<float>(frontend->getWidth()), 40 / static_cast<float>(frontend->getHeight())), _theme(&ThemeType::NONE)
{
	setBorder(true);
}

IUINodeSpriteSelector::~IUINodeSpriteSelector ()
{
}

void IUINodeSpriteSelector::renderSelectorEntry (int index, const SpriteDefPtr& data, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	const SpritePtr& sprite = UI::get().loadSprite(data->id);
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		sprite->render(_frontend, layer, x, y, colWidth, rowHeight, data->angle, alpha);
	}
	if (_selection && data->id == (*_selection)->id) {
		_frontend->renderRect(x, y, colWidth, rowHeight, colorYellow);
	}
}

void IUINodeSpriteSelector::update (uint32_t deltaTime)
{
	Super::update(deltaTime);
	for (SelectorEntryIter i = _entries.begin(); i != _entries.end(); ++i) {
		const SpritePtr& sprite = UI::get().loadSprite((*i)->id);
		sprite->update(deltaTime);
	}
}

void IUINodeSpriteSelector::addSprites (const ThemeType& theme)
{
	_theme = &theme;
	reset();
	for (SpriteDefMapConstIter i = SpriteDefinition::get().begin(); i != SpriteDefinition::get().end(); ++i) {
		const SpriteDefPtr& sprite = i->second;
		if (!sprite->theme.isNone() && sprite->theme != theme)
			continue;
		if (!shouldBeShown(sprite))
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
