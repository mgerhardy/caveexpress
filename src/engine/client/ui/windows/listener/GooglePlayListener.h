#pragma once

#include "engine/common/CommandSystem.h"

class GooglePlayListener: public UINodeListener {
public:
	GooglePlayListener() {
	}

	void onClick() override
	{
		Commands.executeCommandLine("googleplay-connect");
	}
};

