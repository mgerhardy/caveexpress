#pragma once

#include "common/EntityType.h"

namespace EntityTypes {
extern EntityType DECORATION;
extern EntityType SOLID;
extern EntityType GROUND;
extern EntityType LAVA;
extern EntityType NPC_FRIENDLY_GRANDPA;
extern EntityType NPC_FRIENDLY_WOMAN;
extern EntityType NPC_FRIENDLY_MAN;
extern EntityType NPC_FISH;
extern EntityType NPC_FLYING;
extern EntityType NPC_WALKING;
extern EntityType NPC_MAMMUT;
extern EntityType NPC_BLOWING;
extern EntityType PLAYER;
extern EntityType WATER;
extern EntityType CAVE;
extern EntityType WINDOW;
extern EntityType PLATFORM;
extern EntityType STONE;
extern EntityType PACKAGE_ROCK;
extern EntityType PACKAGE_ICE;
extern EntityType EMITTER;
extern EntityType TREE;
extern EntityType APPLE;
extern EntityType BANANA;
extern EntityType EGG;
extern EntityType BORDER;
extern EntityType MODIFICATOR;
extern EntityType PACKAGETARGET_ICE;
extern EntityType PACKAGETARGET_ROCK;
extern EntityType GEYSER_ICE;
extern EntityType GEYSER_ROCK;
extern EntityType BOMB;
extern EntityType PARTICLE;

inline bool isNpcGrandpa (const EntityType& other)
{
	return other == NPC_FRIENDLY_GRANDPA;
}

inline bool isNpcMan (const EntityType& other)
{
	return other == NPC_FRIENDLY_MAN;
}

inline bool isNpcWoman (const EntityType& other)
{
	return other == NPC_FRIENDLY_WOMAN;
}

inline bool isModificator (const EntityType& other)
{
	return other == MODIFICATOR;
}

inline bool isPackageTarget (const EntityType& other)
{
	return other == PACKAGETARGET_ICE || other == PACKAGETARGET_ROCK;
}

inline bool isApple (const EntityType& other)
{
	return other == APPLE;
}

inline bool isBanana (const EntityType& other)
{
	return other == BANANA;
}

inline bool isEgg (const EntityType& other)
{
	return other == EGG;
}

inline bool isGeyser (const EntityType& other)
{
	return other == GEYSER_ICE || other == GEYSER_ROCK;
}

inline bool isBomb (const EntityType& other)
{
	return other == BOMB;
}

inline bool isNpcMammut (const EntityType& other)
{
	return other == NPC_MAMMUT;
}

inline bool isNpcWalking (const EntityType& other)
{
	return other == NPC_WALKING;
}

inline bool isNpcFlying (const EntityType& other)
{
	return other == NPC_FLYING;
}

inline bool isNpcFish (const EntityType& other)
{
	return other == NPC_FISH;
}

inline bool isNpcBlowing (const EntityType& other)
{
	return other == NPC_BLOWING;
}

inline bool isParticle (const EntityType& other)
{
	return other == PARTICLE;
}

inline bool isLava (const EntityType& other)
{
	return other == LAVA;
}

inline bool isWater (const EntityType& other)
{
	return other == WATER;
}

inline bool isPlatform (const EntityType& other)
{
	return other == PLATFORM;
}

inline bool isEmitter (const EntityType& other)
{
	return other == EMITTER;
}

inline bool isBorder (const EntityType& other)
{
	return other == BORDER;
}

inline bool isGround (const EntityType& other)
{
	return other == GROUND;
}

inline bool isDecoration (const EntityType& other)
{
	return other == DECORATION;
}

inline bool isPlayer (const EntityType& other)
{
	return other == PLAYER;
}

inline bool isTree (const EntityType& other)
{
	return other == TREE;
}

inline bool isStone (const EntityType& other)
{
	return other == STONE;
}

inline bool isPackage (const EntityType& other)
{
	return other == PACKAGE_ROCK || other == PACKAGE_ICE;
}

inline bool isWindow (const EntityType& other)
{
	return other == WINDOW;
}

inline bool isCave (const EntityType& other)
{
	return other == CAVE;
}

inline bool isSolid (const EntityType& other)
{
	return other == SOLID || isGround(other) || isGeyser(other) || isPackageTarget(other);
}

inline bool isMapTile (const EntityType& other)
{
	return isDecoration(other) || isWindow(other) || isSolid(other) || isCave(other) || isLava(other);
}

inline bool isNpcAttacking (const EntityType& other)
{
	return isNpcMammut(other) || isNpcWalking(other);
}

inline bool isNpcCave (const EntityType& other)
{
	return isNpcGrandpa(other) || isNpcMan(other) || isNpcWoman(other);
}

inline bool isFruit (const EntityType& other)
{
	return isApple(other) || isBanana(other);
}

inline bool isNpcAggressive (const EntityType& other)
{
	return isNpcAttacking(other) || isNpcFlying(other) || isNpcFish(other);
}

inline bool isNpc (const EntityType& other)
{
	return isNpcCave(other) || isNpcAggressive(other) || isNpcBlowing(other);
}

inline bool isCollectable (const EntityType& other)
{
	return isFruit(other) || isStone(other) || isPackage(other) || isEgg(other) || isBomb(other);
}

inline bool isDynamic (const EntityType& other)
{
	return isNpc(other) || isPlayer(other) || isCollectable(other) || isParticle(other);
}

// returns true if the entity has a direction for the idle animation
inline bool hasDirection (const EntityType& other)
{
	return isNpcAttacking(other) || isNpcFlying(other) || isNpcBlowing(other) || isNpcFish(other);
}

}
