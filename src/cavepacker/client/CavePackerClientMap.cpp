#include "cavepacker/client/CavePackerClientMap.h"
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
#include <SDL.h>

CavePackerClientMap::CavePackerClientMap (int x, int y, int width, int height, IFrontend *frontend,
		ServiceProvider& serviceProvider, int referenceTileWidth) :
		ClientMap(x, y, width, height, frontend, serviceProvider, referenceTileWidth)
{
}

void CavePackerClientMap::start ()
{
}

bool CavePackerClientMap::wantLerp()
{
	return false;
}

void CavePackerClientMap::render (int x, int y) const
{
	ExecutionTime renderTime("ClientMapRender", 2000L);
	const int baseX = x + _x + _camera.getViewportX();
	const int baseY = y + _y + _camera.getViewportY();
	int layerX = baseX;
	int layerY = baseY;
	getLayerOffset(layerX, layerY);

	_frontend->enableScissor(layerX, layerY,
			std::min(getWidth() - layerX, _mapWidth * _scale),
			std::min(getHeight() - layerY, _mapHeight * _scale));
	renderLayer(layerX, layerY, LAYER_BACK);
	renderLayer(layerX, layerY, LAYER_MIDDLE);
	renderLayer(layerX, layerY, LAYER_FRONT);

	if (_restartDue != 0) {
		renderFadeOutOverlay(x, y);
	}

	Config.setDebugRendererData(layerX, layerY, getWidth(), getHeight(), _scale);
	Config.getDebugRenderer().render();

	_particleSystem.render(_frontend, layerX, layerY);

	_frontend->disableScissor();
}
