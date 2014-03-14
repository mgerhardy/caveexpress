#pragma once

#include "engine/common/ICommand.h"
#include "engine/client/ClientMap.h"
#include "engine/common/Direction.h"
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
		if (args.size() != 0) {
			_map.resetAcceleration(_dir);
			return;
		}

		if (!_map.isActive() || _map.isPause())
			return;

		_map.accelerate(_dir);
	}
};
