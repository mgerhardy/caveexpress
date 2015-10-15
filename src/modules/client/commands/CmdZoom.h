#pragma once

#include "common/ICommand.h"
#include "client/ClientMap.h"

class CmdZoom: public ICommand {
private:
	ClientMap& _map;

public:
	CmdZoom (ClientMap& map) :
			_map(map)
	{
	}

	void run (const Args& args) override
	{
		if (args.empty()) {
			return;
		}

		const std::string& arg = *args.begin();
		const float zoom = string::toFloat(arg);
		_map.setZoom(_map.getZoom() + zoom);
	}
};
