#include "CaveExpressEntityType.h"
#include "common/LUALibrary.h"
#include "common/Log.h"
#include "common/String.h"

namespace caveexpress {

namespace EntityTypes {
EntityType DECORATION("decoration");
EntityType SOLID("solid");
EntityType GROUND("ground");
EntityType LAVA("lava");
EntityType NPC_FRIENDLY_GRANDPA("npc-grandpa");
EntityType NPC_FRIENDLY_WOMAN("npc-woman");
EntityType NPC_FRIENDLY_MAN("npc-man");
EntityType NPC_FISH("npc-fish");
EntityType NPC_FLYING("npc-flying");
EntityType NPC_WALKING("npc-walking");
EntityType NPC_MAMMUT("npc-mammut");
EntityType NPC_BLOWING("npc-blowing");
EntityType PLAYER("player");
EntityType WATER("water");
EntityType CAVE("cave");
EntityType WINDOW("window");
EntityType PLATFORM("platform");
EntityType STONE("item-stone");
EntityType PACKAGE_ROCK("item-package");
EntityType PACKAGE_ICE("item-package-ice");
EntityType EMITTER("emitter");
EntityType TREE("tree");
EntityType APPLE("item-apple");
EntityType BANANA("item-banana");
EntityType EGG("item-egg");
EntityType BORDER("border");
EntityType MODIFICATOR("modificator");
EntityType PACKAGETARGET_ICE("tile-packagetarget-ice-01");
EntityType PACKAGETARGET_ROCK("tile-packagetarget-rock-01");
EntityType GEYSER_ICE("tile-geyser-ice-01");
EntityType GEYSER_ROCK("tile-geyser-rock-01");
EntityType GEYSER_JUNGLE("tile-geyser-jungle-01");
EntityType GEYSER_DESERT("tile-geyser-desert-01");
EntityType BOMB("item-bomb");
EntityType PARTICLE("particle");
EntityType GATE("gate");
EntityType PRESSUREPLATE("pressureplate");
}

bool loadEntitySizesFromLua (const std::string& path)
{
	LUA lua;
	if (!lua.load(path)) {
		Log::error(LOG_GAMEIMPL, "could not load %s for entity sizes", path.c_str());
		return false;
	}
	EntityType::TypeMapConstIter i = EntityType::begin();
	for (; i != EntityType::end(); ++i) {
		const std::string& name = string::replaceAll(i->second->name, "-", "");
		const float width = lua.getFloatValue(name + ".width", i->second->width);
		const float height = lua.getFloatValue(name + ".height", i->second->height);
		i->second->setSize(width, height);
	}
	return true;
}

}
