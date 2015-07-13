#pragma once

#include "ui/nodes/IUINodeMapEditor.h"
#include "caveexpress/shared/CaveExpressSpriteType.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressAnimation.h"

namespace caveexpress {

class UINodeMapEditor: public IUINodeMapEditor {
private:
	// the height in grid tiles from the bottom of the map
	float _waterHeight;
	void renderWater (int x, int y) const;
	bool placeCave (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			MapEditorLayer layer, int delay, bool overwrite);
	void setFlyingNpc (bool flyingNpc);
	void setFishNpc (bool fishNpc);
	void setPackageTransferCount (int amount);
	void setWaterParameters (float waterHeight, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay);
	void setGravity (float gravity);

	virtual MapEditorLayer getLayer (const SpriteType& type) const override;
	virtual void renderPlayer (int x, int y) const override;
	virtual void renderHighlightItem (int x, int y) const override;
	virtual bool isMapTile (const SpriteType &type) const override;
	virtual bool isPlayer (const EntityType &type) const override;
	virtual IMapContext* getContext (const std::string& mapname) override;
	virtual const Animation& getEmitterAnimation (const EntityType& type) const override;
	virtual void setState (const State& state) override;
	virtual void loadFromContext (IMapContext& ctx) override;
	virtual void doClear () override;
	virtual bool save () override;
	virtual bool placeTileItem (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			MapEditorLayer layer, bool overwrite, EntityAngle angle) override;
	virtual bool placeEmitter (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			int emitterAmount, int emitterDelay, bool overwrite, EntityAngle angle, const std::string& settings) override;
	virtual bool isOverlapping (const TileItem& item1, const TileItem& item2) const override;
public:
	UINodeMapEditor (IFrontend *frontend, IMapManager& mapManager);
	virtual ~UINodeMapEditor ();

	void autoFill (const ThemeType& theme);

	void setWaterHeight (float waterHeight);
};

inline void UINodeMapEditor::setGravity (float gravity)
{
	setSetting(msn::GRAVITY, string::toString(gravity));
}

inline void UINodeMapEditor::setWaterHeight (float waterHeight)
{
	if (_waterHeight > _mapHeight) {
		_waterHeight = string::toFloat(msd::WATER_HEIGHT);
	} else {
		_waterHeight = waterHeight;
	}
	setSetting(msn::WATER_HEIGHT, string::toString(waterHeight));
}

inline void UINodeMapEditor::setWaterParameters (float waterHeight, float waterChangeSpeed,
		uint32_t waterRisingDelay, uint32_t waterFallingDelay)
{
	setWaterHeight(waterHeight);
	setSetting(msn::WATER_CHANGE, string::toString(waterChangeSpeed));
	setSetting(msn::WATER_RISING_DELAY, string::toString(waterRisingDelay));
	setSetting(msn::WATER_FALLING_DELAY, string::toString(waterFallingDelay));
}

inline void UINodeMapEditor::setFlyingNpc (bool flyingNpc)
{
	setSetting(msn::FLYING_NPC, flyingNpc ? "true" : "false");
}

inline void UINodeMapEditor::setFishNpc (bool fishNpc)
{
	setSetting(msn::FISH_NPC, fishNpc ? "true" : "false");
}

inline void UINodeMapEditor::setPackageTransferCount (int amount)
{
	setSetting(msn::PACKAGE_TRANSFER_COUNT, string::toString(amount));
}

}
