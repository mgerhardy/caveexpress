#include "miniracer/client/MiniRacerClientMap.h"
#include "miniracer/shared/EntityStates.h"
#include "miniracer/shared/network/messages/ProtocolMessages.h"
#include "particles/Sparkle.h"
#include "common/MapSettings.h"
#include "network/messages/StopMovementMessage.h"
#include "network/messages/MovementMessage.h"
#include "network/messages/FingerMovementMessage.h"
#include "network/messages/ClientInitMessage.h"
#include "client/entities/ClientMapTile.h"
#include "ui/UI.h"
#include "common/IFrontend.h"
#include "network/ProtocolHandlerRegistry.h"
#include "sound/Sound.h"
#include "common/ConfigManager.h"
#include "common/EventHandler.h"
#include "service/ServiceProvider.h"
#include "common/ExecutionTime.h"
#include "common/DateUtil.h"
#include "miniracer/shared/MiniRacerAnimation.h"
#include "miniracer/shared/MiniRacerEntityType.h"
#include <SDL.h>

namespace miniracer {

MiniRacerClientMap::MiniRacerClientMap (int x, int y, int width, int height, IFrontend *frontend,
		ServiceProvider& serviceProvider, int referenceTileWidth) :
		ClientMap(x, y, width, height, frontend, serviceProvider, referenceTileWidth)
{
}

}
