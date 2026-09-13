#pragma once

#include "network/IProtocolHandler.h"
#include "client/ClientMap.h"
#include "ui/UI.h"
#include "ui/nodes/IUINodeMap.h"

class PlayerListHandler: public ClientProtocolHandler<PlayerListMessage> {
private:
	IUINodeMap* _mapNode;
public:
	PlayerListHandler (IUINodeMap* mapNode) :
			_mapNode(mapNode)
	{
	}
	void execute (const PlayerListMessage* msg) override
	{
		std::vector<lobby::PlayerEntry> list;
		const std::vector<std::string>& names = msg->getList();
		list.reserve(names.size());
		for (size_t i = 0; i < names.size(); ++i) {
			lobby::PlayerEntry entry;
			entry.name = names[i];
			entry.colorIndex = msg->getColorIndex(i);
			list.push_back(entry);
		}
		_mapNode->setPlayerList(list);
	}
};
