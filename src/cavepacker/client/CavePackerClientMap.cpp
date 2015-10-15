#include "cavepacker/client/CavePackerClientMap.h"
#include "cavepacker/shared/EntityStates.h"
#include "cavepacker/shared/network/messages/ProtocolMessages.h"
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
#include "cavepacker/shared/CavePackerAnimation.h"
#include "cavepacker/shared/CavePackerEntityType.h"
#include <SDL.h>

namespace cavepacker {

CavePackerClientMap::CavePackerClientMap(int x, int y, int width, int height,
		IFrontend *frontend, ServiceProvider& serviceProvider, int referenceTileWidth) :
		ClientMap(x, y, width, height, frontend, serviceProvider, referenceTileWidth), _target(nullptr), _targetEnts(0u) {
	_deadlockOverlay = UI::get().loadSprite("deadlock");
}

void CavePackerClientMap::onWindowResize () {
	_target = nullptr;
}

void CavePackerClientMap::setZoom (const float zoom) {
	ClientMap::setZoom(zoom);
	_target = nullptr;
}

void CavePackerClientMap::renderLayer (int x, int y, Layer layer) const {
	bool renderToTexture = false;
	if (layer == LAYER_BACK) {
		if (_target == nullptr || _targetEnts != _entities.size()) {
			_targetEnts = _entities.size();
			_target = _frontend->renderToTexture(x, y, getWidth() * _zoom, getHeight() * _zoom);
			renderToTexture = true;
		}
	} else if (layer == LAYER_MIDDLE && _target) {
		_frontend->renderTarget(_target);
	}
	ClientMap::renderLayer(x, y, layer);
	if (renderToTexture)
		_frontend->disableRenderTarget(_target);
}

void CavePackerClientMap::start() {
	ClientMap::start();
}

void CavePackerClientMap::undo() {
	_serviceProvider.getNetwork().sendToServer(UndoMessage());
}

void CavePackerClientMap::setDeadlocks(const std::vector<int>& _deadlocks) {
	std::vector<int> deadlocks(_deadlocks);
	for (ClientEntityMapIter i = _entities.begin(); i != _entities.end(); ++i) {
		ClientEntityPtr entity = i->second;
		const EntityType& type = entity->getType();
		if (!EntityTypes::isGround(type)) {
			continue;
		}
		ClientMapTile* tile = static_cast<ClientMapTile*>(entity);
		const vec2& pos = tile->getPos();
		const int col = pos.x + 0.5f;
		const int row = pos.y + 0.5f;
		const int index = col + _mapWidth * row;
		auto iter = std::find(deadlocks.begin(), deadlocks.end(), index);
		if (iter == deadlocks.end()) {
			tile->removeOverlay(_deadlockOverlay);
			continue;
		}
		deadlocks.erase(iter);
		tile->addOverlay(_deadlockOverlay);
	}
}

void CavePackerClientMap::update(uint32_t deltaTime) {
	ClientMap::update(deltaTime);
	for (ClientEntityMapIter i = _entities.begin(); i != _entities.end(); ++i) {
		ClientEntityPtr entity = i->second;
		const EntityType& type = entity->getType();
		auto state = entity->getState();
		if (EntityTypes::isPackage(type)) {
			if (state == CavePackerEntityStates::DELIVERED) {
				entity->setAnimationType(Animations::DELIVERED);
			} else if (state == CavePackerEntityStates::DEADLOCK) {
				entity->setAnimationType(Animations::DEADLOCK);
			} else {
				entity->setAnimationType(Animation::NONE);
			}
		}
	}
}

}
