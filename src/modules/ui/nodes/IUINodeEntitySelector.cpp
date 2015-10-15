#include <ui/nodes/IUINodeEntitySelector.h>
#include "common/Log.h"
#include "ui/UI.h"
#include "common/SpriteDefinition.h"

IUINodeEntitySelector::IUINodeEntitySelector (IFrontend *frontend, int cols, int rows) :
		Super(frontend, cols, rows, 40 / static_cast<float>(frontend->getWidth()), 40 / static_cast<float>(frontend->getHeight())), _theme(&ThemeType::NONE)
{
	_renderBorder = true;
	setPadding(0.0f);
	setColSpacing(0);
	setRowSpacing(0);
}

IUINodeEntitySelector::~IUINodeEntitySelector ()
{
}

void IUINodeEntitySelector::renderSelectorEntry (int index, const EntityTypeWrapper& data, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		data.sprite->render(_frontend, layer, x, y, colWidth, rowHeight, data.angle, alpha);
	}
	if (_selection && data.type->id == _selection->type->id) {
		_frontend->renderRect(x, y, colWidth, rowHeight, colorYellow);
	}
}

void IUINodeEntitySelector::update (uint32_t deltaTime)
{
	Super::update(deltaTime);
	for (SelectorEntryIter i = _entries.begin(); i != _entries.end(); ++i) {
		i->sprite->update(deltaTime);
	}
}

void IUINodeEntitySelector::addEntity (const EntityType& type, const ThemeType& theme)
{
	const Animation& animation = getAnimation(type);
	const SpriteDefPtr& def = SpriteDefinition::get().getFromEntityType(type, animation);
	if (!def)
		return;
	if (!def->theme.isNone() && def->theme != theme)
		return;
	const SpritePtr& sprite = UI::get().loadSprite(def->id);
	const EntityTypeWrapper w = { &type, sprite, def->angle };
	addData(w);
}

void IUINodeEntitySelector::addEntities (const ThemeType& theme)
{
	_theme = &theme;
	reset();
	for (auto i = EntityType::begin(); i != EntityType::end(); ++i) {
		if (!shouldBeShown(*i->second))
			continue;
		addEntity(*i->second, theme);
	}

	if (_entries.empty()) {
		_selection = nullptr;
		_selectedIndex = -1;
		return;
	}

	_selectedIndex = 0;
	select();
}
