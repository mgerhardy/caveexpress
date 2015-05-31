#pragma once

#include "common/ICommand.h"
#include "common/CommandSystem.h"
#include "caveexpress/shared/constants/Commands.h"
#include "common/IMap.h"
#include <stdint.h>

class CmdMapOpenInEditor: public ICommand {
private:
	IMap& _map;

public:
	CmdMapOpenInEditor (IMap& map) :
			_map(map)
	{
	}

	void run (const Args& args) override
	{
		if (!_map.isActive())
			return;

		const std::string& name = _map.getName();
		Commands.executeCommandLine(CMD_LOADMAP " " + name);
	}
};
