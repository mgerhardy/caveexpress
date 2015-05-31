#include "UINodeEntitySelector.h"
#include "common/Logger.h"
#include "ui/UI.h"
#include "caveexpress/server/entities/EntityEmitter.h"
#include "common/SpriteDefinition.h"

UINodeEntitySelector::UINodeEntitySelector (IFrontend *frontend, int cols, int rows) :
		UINodeSelector<EntityTypeWrapper>(frontend, cols, rows, 40 / static_cast<float>(frontend->getWidth()), 40 / static_cast<float>(frontend->getHeight())), _theme(&ThemeType::NONE)
{
	_renderBorder = true;
	setPadding(0.0f);
	setColSpacing(0);
	setRowSpacing(0);
}

UINodeEntitySelector::~UINodeEntitySelector ()
{
}

void UINodeEntitySelector::renderSelectorEntry (int index, const EntityTypeWrapper& data, int x, int y, int colWidth, int rowHeight, float alpha) const
{
	for (Layer layer = LAYER_BACK; layer < MAX_LAYERS; ++layer) {
		data.sprite->render(_frontend, layer, x, y, colWidth, rowHeight, data.angle, alpha);
	}
	if (_selection && data.type->id == _selection->type->id) {
		_frontend->renderRect(x, y, colWidth, rowHeight, colorYellow);
	}
}

void UINodeEntitySelector::update (uint32_t deltaTime)
{
	UINodeSelector<EntityTypeWrapper>::update(deltaTime);
	for (SelectorEntryIter i = _entries.begin(); i != _entries.end(); ++i) {
		i->sprite->update(deltaTime);
	}
}

void UINodeEntitySelector::addEntity (const EntityType& type, const ThemeType& theme)
{
	const Animation& animation = EntityTypes::hasDirection(type) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE;
	const SpriteDefPtr& def = SpriteDefinition::get().getFromEntityType(type, animation);
	if (!def)
		return;
	if (!def->theme.isNone() && def->theme != theme)
		return;
	const SpritePtr& sprite = UI::get().loadSprite(def->id);
	const EntityTypeWrapper w = { &type, sprite, def->angle };
	addData(w);
}

void UINodeEntitySelector::addEntities (const ThemeType& theme)
{
	_theme = &theme;
	reset();
	addEntity(EntityTypes::PLAYER, theme);
	const EntityType** types = EntityEmitter::getSupportedEntityTypes();
	for (; *types; ++types) {
		addEntity(**types, theme);
	}

	if (_entries.empty()) {
		_selection = nullptr;
		_selectedIndex = -1;
		return;
	}

	_selectedIndex = 0;
	select();
}
