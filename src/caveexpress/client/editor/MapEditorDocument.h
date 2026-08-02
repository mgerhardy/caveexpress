#pragma once

#include "ui/editor/IMapEditorDocument.h"

namespace caveexpress {

class MapEditorDocument: public IMapEditorDocument {
private:
	float _waterHeight = 0.0f;
	int _caveDelay = 5000;

	bool placeCave (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			MapEditorLayer layer, int delay, bool overwrite);
	void setWaterParameters (float waterHeight, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay);

protected:
	MapEditorLayer getLayer (const SpriteType& type) const override;
	bool isMapTileType (const SpriteType& type) const override;
	bool isPlayerType (const EntityType& type) const override;
	const Animation& getEmitterAnimation (const EntityType& type) const override;
	bool shouldSaveTile (const MapEditorTileItem& tile) const override;
	bool shouldSaveEmitter (const MapEditorTileItem& tile) const override;
	void doClear () override;
	void onAfterStateRestored () override;
	bool placeBrushItem (bool overwrite) override;
	void prepareContextForSaving (IMapContext& ctx) override;
	void loadFromContext (IMapContext& ctx) override;
	std::unique_ptr<IMapContext> createContext (const std::string& mapName) const override;
	void fillTilePalette (std::vector<SpriteDefPtr>& out) const override;
	void fillEntityPalette (std::vector<const EntityType*>& out) const override;
	bool isOverlapping (const MapEditorTileItem& item1, const MapEditorTileItem& item2) const override;

public:
	explicit MapEditorDocument (IMapManager& mapManager);

	void setActiveEntityRight (bool right) override;
	void rotateBrush () override;
	void setWaterHeight (float waterHeight) override;
	float getWaterHeight () const override { return _waterHeight; }
	void setCaveDelay (int delay) { _caveDelay = delay; }
	int getCaveDelay () const { return _caveDelay; }
	void changeMapTheme (const ThemeType& toTheme) override;
	void autoFill (const ThemeType& theme) override;
	bool supportsThemeControls () const override { return true; }
	bool supportsWater () const override { return true; }
	bool supportsEmitterParams () const override { return true; }
	const EntityType& getPlayerEntityType () const override;
};

}
