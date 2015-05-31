#pragma once

#include "common/ICommand.h"
#include "client/ClientMap.h"
#include "common/Direction.h"
#include <stdint.h>

class CmdMove: public ICommand {
private:
	ClientMap& _map;
	const Direction _dir;

public:
	CmdMove (ClientMap& map, Direction dir) :
			_map(map), _dir(dir)
	{
	}

	void run (const Args& args) override
	{
		if (!args.empty()) {
			_map.resetAcceleration(_dir);
			return;
		}

		if (!_map.isActive() || _map.isPause())
			return;

		_map.accelerate(_dir);
	}
};
