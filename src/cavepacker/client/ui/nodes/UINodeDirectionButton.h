#pragma once

#include "ui/nodes/UINodeButton.h"
#include "common/CommandSystem.h"

namespace cavepacker {

/**
 * On-screen direction pad button that holds +move_* while pressed and sends the
 * matching release command on finger/mouse up so the server can auto-repeat steps.
 */
class UINodeDirectionButton: public UINodeButton {
private:
	std::string _moveCommand;
public:
	UINodeDirectionButton (IFrontend *frontend, const std::string& moveCommand) :
			UINodeButton(frontend), _moveCommand(moveCommand)
	{
	}

	bool onFingerPress (int64_t finger, uint16_t x, uint16_t y) override
	{
		UINodeButton::onFingerPress(finger, x, y);
		Commands.executeCommandLine("+" + _moveCommand);
		return true;
	}

	bool onFingerRelease (int64_t finger, uint16_t x, uint16_t y, bool motion) override
	{
		_fingerPressed = false;
		Commands.executeCommandLine(_moveCommand + " -");
		return true;
	}

	bool onMouseLeftPress (int32_t x, int32_t y) override
	{
		UINodeButton::onMouseLeftPress(x, y);
		Commands.executeCommandLine("+" + _moveCommand);
		return true;
	}

	bool onMouseLeftRelease (int32_t x, int32_t y) override
	{
		_mousePressed = false;
		Commands.executeCommandLine(_moveCommand + " -");
		return true;
	}
};

}
