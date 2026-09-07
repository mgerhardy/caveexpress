#include "caveexpress/client/ClientEntityFactories.h"
#include "caveexpress/client/entities/ClientCaveTile.h"
#include "caveexpress/client/entities/ClientGate.h"
#include "caveexpress/client/entities/ClientNPC.h"
#include "caveexpress/client/entities/ClientParticle.h"
#include "caveexpress/client/entities/ClientPressurePlate.h"
#include "caveexpress/client/entities/ClientWindowTile.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "client/entities/ClientEntity.h"
#include "client/entities/ClientEntityFactory.h"
#include "client/entities/ClientMapTile.h"
#include "client/entities/ClientPlayer.h"
#include "common/Singleton.h"

namespace caveexpress {

void registerClientEntityFactories ()
{
	ClientEntityRegistry &r = Singleton<ClientEntityRegistry>::getInstance();
	r.registerFactory(&EntityTypes::DECORATION, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::SOLID, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::LAVA, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::GROUND, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::CAVE, ClientCaveTile::FACTORY);
	r.registerFactory(&EntityTypes::WINDOW, ClientWindowTile::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FRIENDLY_GRANDPA, ClientNPC::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FRIENDLY_WOMAN, ClientNPC::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FRIENDLY_MAN, ClientNPC::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FISH, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_FLYING, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_WALKING, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_MAMMUT, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::NPC_BLOWING, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PLAYER, ClientPlayer::FACTORY);
	r.registerFactory(&EntityTypes::STONE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::TREE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGE_ICE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGE_ROCK, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGETARGET_ICE, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGETARGET_ROCK, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::APPLE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::BANANA, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::EGG, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PARTICLE, ClientParticle::FACTORY);
	r.registerFactory(&EntityTypes::GEYSER_ICE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::GEYSER_ROCK, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::GEYSER_JUNGLE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::GEYSER_DESERT, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::BOMB, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::GATE, ClientGate::FACTORY);
	r.registerFactory(&EntityTypes::PRESSUREPLATE, ClientPressurePlate::FACTORY);
}

}
