#pragma once

#include "common/ConfigManager.h"
#include "ui/nodes/UINode.h"

class GameControllerTriggerNodeListener : public UINodeListener {
private:
	bool _on;

public:
	GameControllerTriggerNodeListener(bool on) : _on(on) {
	}

	void onClick() override {
		const bool state = Config.isGameControllerTriggerActive();
		if (_on == state)
			return;

		Config.toggleGameControllerTrigger();
	}
};
