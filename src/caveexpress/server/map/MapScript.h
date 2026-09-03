#pragma once

namespace caveexpress {

class CaveExpressMapContext;

/**
 * @brief Registers runtime Lua bindings for a live map (spawn/control entities, finish, keys).
 * Call after @c CaveExpressMapContext::setRuntimeMap and before @c onMapLoaded.
 *
 * Script surface includes: onUpdate/onMapLoaded hooks, spawn*, replaceTile/removeTileAt,
 * cave light/spawn helpers, setWaterHeight, input lock, client-fed isKeyPressed/skip, finish,
 * entity physics (setVelocity/applyImpulse/applyForce/setGravityScale), calculateVelocity.
 */
class MapScript {
public:
	static void install (CaveExpressMapContext& ctx);
};

}
