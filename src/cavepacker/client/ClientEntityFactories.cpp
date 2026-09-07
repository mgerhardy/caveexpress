#include "cavepacker/client/ClientEntityFactories.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include "client/entities/ClientEntity.h"
#include "client/entities/ClientEntityFactory.h"
#include "client/entities/ClientMapTile.h"
#include "client/entities/ClientPlayer.h"
#include "common/Singleton.h"

namespace cavepacker {

void registerClientEntityFactories ()
{
	ClientEntityRegistry &r = Singleton<ClientEntityRegistry>::getInstance();
	r.registerFactory(&EntityTypes::SOLID, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::GROUND, ClientMapTile::FACTORY);
	r.registerFactory(&EntityTypes::PACKAGE, ClientEntity::FACTORY);
	r.registerFactory(&EntityTypes::PLAYER, ClientPlayer::FACTORY);
	r.registerFactory(&EntityTypes::TARGET, ClientMapTile::FACTORY);
}

}
