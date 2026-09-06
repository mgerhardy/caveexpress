#pragma once

#include "ui/editor/IMapEditorDocument.h"
#include <string>
#include <vector>

namespace cavepacker {

class MapEditorDocument: public IMapEditorDocument {
protected:
	MapEditorLayer getLayer (const SpriteType& type) const override;
	bool isMapTileType (const SpriteType& type) const override;
	bool isPlayerType (const EntityType& type) const override;
	const Animation& getEmitterAnimation (const EntityType& type) const override;
	bool canPlaceTileItem (const MapEditorTileItem& item) const override;
	bool shouldSaveTile (const MapEditorTileItem& tile) const override;
	bool shouldSaveEmitter (const MapEditorTileItem& tile) const override;
	bool placeBrushItem (bool overwrite) override;
	bool eraseAtSelection (bool recordUndo = true) override;
	void loadFromContext (IMapContext& ctx) override;
	void onAfterStateRestored () override;
	std::unique_ptr<IMapContext> createContext (const std::string& mapName) const override;
	void fillTilePalette (std::vector<SpriteDefPtr>& out) const override;
	void fillEntityPalette (std::vector<const EntityType*>& out) const override;
	bool isOverlapping (const MapEditorTileItem& item1, const MapEditorTileItem& item2) const override;
	bool floodFillCanPaint (int x, int y, const SpriteDefPtr& brush) const override;

	bool isWallItem (const MapEditorTileItem& item) const;
	bool isPackageItem (const MapEditorTileItem& item) const;
	bool isTargetItem (const MapEditorTileItem& item) const;
	bool isGroundItem (const MapEditorTileItem& item) const;
	bool isWallAt (int col, int row) const;
	bool isPlayableAt (int col, int row) const;
	bool sameCell (const MapEditorTileItem& a, const MapEditorTileItem& b) const;
	void tagPackageItems ();
	bool findOpenCell (int& x, int& y) const;

public:
	explicit MapEditorDocument (IMapManager& mapManager);
	const EntityType& getPlayerEntityType () const override;
	std::string getUserMapsPath () const override;
	std::string getGameDataMapsPath () const override;

	int countWalls () const;
	int countGrounds () const;
	int countPackages () const;
	int countTargets () const;
	void autoTileWalls (bool recordUndo = false);
	void makePlayable ();
	char cellGlyphAt (int col, int row) const;
	bool campaignContainsExact (const std::string& campaignFile) const;
	void collectGameValidationIssues (std::vector<std::string>& out) const override;
	bool evaluateReachability (int& reachablePlayable, int& playableCells, std::string& failure) const;

	bool addToCampaign (const std::string& campaignFile);
	bool removeFromCampaign (const std::string& campaignFile);
	bool createCampaign (const std::string& campaignFile, const std::string& campaignId, const std::string& text);
	std::vector<std::string> listCampaignFiles () const;
	std::vector<std::string> campaignsContainingMap () const;
	std::vector<std::string> listMapsInCampaign (const std::string& campaignFile) const;
};

}
