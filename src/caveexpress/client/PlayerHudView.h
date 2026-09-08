#pragma once

#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"
#include "common/EntityType.h"
#include "common/Math.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/windows/IUIMapWindow.h"
#include "common/SpriteDefinition.h"
#include "common/String.h"

namespace caveexpress {

struct PlayerHudState {
	uint16_t hitpoints = 0;
	uint8_t lives = 0;
	uint8_t targetCave = 0;
	uint8_t collectedTypeId = 0;

	bool operator== (const PlayerHudState& o) const
	{
		return hitpoints == o.hitpoints && lives == o.lives && targetCave == o.targetCave
				&& collectedTypeId == o.collectedTypeId;
	}
};

/** Apply another player's HUD widgets. Flash only when values change (caller decides). */
class PlayerHudView {
public:
	static void applyHitpoints (uint16_t hitpoints, bool flash)
	{
		UINodeBar* bar = UI::get().setBarValue(UI_WINDOW_MAP, UINODE_HITPOINTS, hitpoints);
		if (!bar)
			return;
		const bool red = hitpoints < 30;
		const bool yellow = hitpoints < 60;
		const Color& colorValue = red ? colorRed : yellow ? colorYellow : colorGreen;
		Color color;
		Vector4Set(colorValue, color);
		bar->setBarColor(color);
		bar->setBorderColor(color);
		if (flash)
			bar->flash(2000, red ? 2 : 1);
	}

	static void applyLives (uint8_t lives, bool flash)
	{
		UINodeSprite* node = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_LIVES);
		if (!node)
			return;
		node->clearSprites();
		const SpritePtr sprite = UI::get().loadSprite("icon-heart");
		for (uint8_t i = 0; i < lives; ++i) {
			node->addSprite(sprite);
		}
		if (flash)
			node->flash();
	}

	static void applyCollected (uint8_t collectedTypeId, bool flash)
	{
		UINodeSprite* node = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_COLLECTED);
		if (!node)
			return;
		const EntityType& type = EntityType::get(collectedTypeId);
		if (collectedTypeId == 0 || type.isNone()) {
			node->clearSprites();
			return;
		}
		node->clearSprites();
		const Animation& animation = EntityTypes::hasDirection(type) ? Animations::ANIMATION_IDLE_RIGHT : Animations::ANIMATION_IDLE;
		const std::string name = SpriteDefinition::get().getSpriteName(type, animation);
		const SpritePtr& sprite = UI::get().loadSprite(name);
		node->addSprite(sprite);
		if (flash)
			node->flash();
	}

	static void applyTargetCave (uint8_t caveNumber)
	{
		UINodeSprite* node = UI::get().getNode<UINodeSprite>(UI_WINDOW_MAP, UINODE_TARGETCAVEID);
		if (!node)
			return;
		node->clearSprites();
		if (caveNumber == 0)
			return;
		if (caveNumber >= 100) {
			const SpritePtr& sprite = UI::get().loadSprite("item-stone-idle");
			node->addSprite(sprite);
			return;
		}
		const std::string caveNumberStr = "cavenumber" + string::toString((int)caveNumber);
		const SpritePtr& sprite = UI::get().loadSprite(caveNumberStr);
		node->addSprite(sprite);
	}

	static void apply (const PlayerHudState& state, bool flash)
	{
		applyHitpoints(state.hitpoints, flash);
		applyLives(state.lives, flash);
		applyCollected(state.collectedTypeId, flash);
		applyTargetCave(state.targetCave);
	}
};

}
