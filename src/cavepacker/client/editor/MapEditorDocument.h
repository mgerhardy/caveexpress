#pragma once

#include "ui/editor/IMapEditorDocument.h"

namespace cavepacker {

class MapEditorDocument: public IMapEditorDocument {
protected:
	MapEditorLayer getLayer (const SpriteType& type) const override;
	bool isMapTileType (const SpriteType& type) const override;
	bool isPlayerType (const EntityType& type) const override;
	const Animation& getEmitterAnimation (const EntityType& type) const override;
	std::unique_ptr<IMapContext> createContext (const std::string& mapName) const override;
	void fillTilePalette (std::vector<SpriteDefPtr>& out) const override;
	void fillEntityPalette (std::vector<const EntityType*>& out) const override;
	bool placeBrushItem (bool overwrite) override;

public:
	explicit MapEditorDocument (IMapManager& mapManager);
	const EntityType& getPlayerEntityType () const override;
};

}
