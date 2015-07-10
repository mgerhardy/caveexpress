#pragma once

#include "common/ICommand.h"
#include "miniracer/client/MiniRacerClientMap.h"
#include <stdint.h>

namespace miniracer {

class CmdUndo: public ICommand {
private:
	MiniRacerClientMap& _map;

public:
	CmdUndo (MiniRacerClientMap& map) :
			_map(map)
	{
	}

	void run (const Args& args) override
	{
		_map.undo();
	}
};

}
