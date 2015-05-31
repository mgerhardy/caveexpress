#include "cavepacker/client/CavePackerClientMap.h"
#include "cavepacker/shared/EntityStates.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"
#include "client/particles/Sparkle.h"
#include "common/MapSettings.h"
#include "common/network/messages/StopMovementMessage.h"
#include "common/network/messages/MovementMessage.h"
#include "common/network/messages/FingerMovementMessage.h"
#include "common/network/messages/ClientInitMessage.h"
#include "client/entities/ClientMapTile.h"
#include "client/ui/UI.h"
#include "common/IFrontend.h"
#include "common/network/ProtocolHandlerRegistry.h"
#include "client/sound/Sound.h"
#include "common/ConfigManager.h"
#include "common/EventHandler.h"
#include "common/ServiceProvider.h"
#include "client/GLFunc.h"
#include "client/GLShared.h"
#include "common/ExecutionTime.h"
#include "common/DateUtil.h"
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
	ClientMap::start();
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
