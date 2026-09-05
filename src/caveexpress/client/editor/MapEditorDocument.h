#pragma once

#include "ui/editor/IMapEditorDocument.h"
#include "caveexpress/shared/MapValidator.h"
#include <string>
#include <vector>

namespace caveexpress {

struct UnpairedTrigger {
	std::string kind;
	std::string linkId;
	gridCoord x = 0.0f;
	gridCoord y = 0.0f;
};

class MapEditorDocument: public IMapEditorDocument {
private:
	float _waterHeight = 0.0f;
	int _caveDelay = 5000;
	const EntityType* _caveNpcType = &EntityType::NONE;
	bool _pickGateTarget = false;
	gridCoord _linkPlateX = 0.0f;
	gridCoord _linkPlateY = 0.0f;

	bool placeCave (const SpriteDefPtr& def, const EntityType* entityType, gridCoord gridX, gridCoord gridY,
			MapEditorLayer layer, int delay, bool overwrite);
	void setWaterParameters (float waterHeight, float waterChangeSpeed, uint32_t waterRisingDelay, uint32_t waterFallingDelay);

	bool requiresBackgroundTile (const SpriteType& type) const;
	bool isHangingGroundSprite (const SpriteDefPtr& def) const;
	bool hasBackgroundCovering (gridCoord gridX, gridCoord gridY, gridSize width, gridSize height) const;
	bool hasAirBelow (gridCoord gridX, gridCoord gridY) const;
	bool hasBridgeSideNeighbors (gridCoord gridX, gridCoord gridY) const;

protected:
	MapEditorLayer getLayer (const SpriteType& type) const override;
	bool isMapTileType (const SpriteType& type) const override;
	bool isPlayerType (const EntityType& type) const override;
	const Animation& getEmitterAnimation (const EntityType& type) const override;
	bool canPlaceTileItem (const MapEditorTileItem& item) const override;
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
	void rotateSelectionOrBrush () override;
	void setWaterHeight (float waterHeight) override;
	float getWaterHeight () const override { return _waterHeight; }
	void setCaveDelay (int delay) { _caveDelay = delay; }
	int getCaveDelay () const { return _caveDelay; }
	void setCaveNpcType (const EntityType& type) { _caveNpcType = &type; }
	const EntityType& getCaveNpcType () const { return *_caveNpcType; }
	void changeMapTheme (const ThemeType& toTheme) override;
	void previewThemeRemap (const ThemeType& toTheme, int& wouldReplace, int& leftoverThemed) const;
	void autoFill (const ThemeType& theme) override;
	void makePlayable ();
	void collectGameValidationIssues (std::vector<std::string>& out) const override;
	MapMetrics evaluateLayout () const;
	bool addToCampaign (const std::string& campaignFile);
	bool removeFromCampaign (const std::string& campaignFile);
	bool createCampaign (const std::string& campaignFile, const std::string& campaignId, const std::string& text);
	std::vector<std::string> listCampaignFiles () const;
	std::vector<std::string> campaignsContainingMap () const;
	std::vector<std::string> listMapsInCampaign (const std::string& campaignFile) const;
	std::vector<UnpairedTrigger> listUnpairedTriggers () const;
	bool supportsThemeControls () const override { return true; }
	bool supportsWater () const override { return true; }
	bool supportsEmitterParams () const override { return true; }
	bool supportsMapScript () const override { return true; }
	const EntityType& getPlayerEntityType () const override;

	void beginPickGateTarget ();
	bool isPickingGateTarget () const { return _pickGateTarget; }
	void cancelPickGateTarget () { _pickGateTarget = false; }
	MapEditorTileItem* findTileAt (gridCoord gridX, gridCoord gridY);
	const MapEditorTileItem* findLinkedPartner (const MapEditorTileItem& item) const;
};

}
