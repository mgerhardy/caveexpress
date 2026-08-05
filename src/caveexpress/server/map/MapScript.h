#pragma once

namespace caveexpress {

class CaveExpressMapContext;

/**
 * @brief Registers runtime Lua bindings for a live map (spawn/control entities, finish, keys).
 * Call after @c CaveExpressMapContext::setRuntimeMap and before @c onMapLoaded.
 */
class MapScript {
public:
	static void install (CaveExpressMapContext& ctx);
};

}
