#include "cavepacker/client/CavePackerClientMap.h"
#include "cavepacker/shared/EntityStates.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"
#include "engine/client/particles/Sparkle.h"
#include "engine/common/MapSettings.h"
#include "engine/common/network/messages/StopMovementMessage.h"
#include "engine/common/network/messages/MovementMessage.h"
#include "engine/common/network/messages/FingerMovementMessage.h"
#include "engine/common/network/messages/ClientInitMessage.h"
#include "engine/client/entities/ClientMapTile.h"
#include "engine/client/ui/UI.h"
#include "engine/common/IFrontend.h"
#include "engine/common/network/ProtocolHandlerRegistry.h"
#include "engine/client/sound/Sound.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/EventHandler.h"
#include "engine/common/ServiceProvider.h"
#include "engine/common/GLShared.h"
#include "engine/common/ExecutionTime.h"
#include "engine/common/DateUtil.h"
#include "cavepacker/shared/CavePackerAnimation.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include <SDL.h>

CavePackerClientMap::CavePackerClientMap (int x, int y, int width, int height, IFrontend *frontend,
		ServiceProvider& serviceProvider, int referenceTileWidth) :
		ClientMap(x, y, width, height, frontend, serviceProvider, referenceTileWidth)
{
}

void CavePackerClientMap::start ()
{
}

void CavePackerClientMap::undo ()
{
	_serviceProvider.getNetwork().sendToServer(UndoMessage());
}

void CavePackerClientMap::update (uint32_t deltaTime)
{
	ClientMap::update(deltaTime);
	for (ClientEntityMapIter i = _entities.begin(); i != _entities.end(); ++i) {
		if (i->second->getType() != EntityTypes::PACKAGE)
			continue;
		const bool delivered = i->second->getState() == CavePackerEntityStates::DELIVERED;
		if (delivered) {
			i->second->setAnimationType(Animations::DELIVERED);
		} else {
			i->second->setAnimationType(Animation::NONE);
		}
	}
}
