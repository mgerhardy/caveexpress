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

		const String& arg = *args.begin();
		const float zoom = arg.toFloat();
		_map.setZoom(_map.getZoom() + zoom);
	}
};
