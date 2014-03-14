#pragma once

class JoystickNodeListener: public UINodeListener {
private:
	bool _on;
public:
	JoystickNodeListener(bool on) :
			_on(on) {
	}

	void onClick() override
	{
		const bool state = Config.isJoystick();
		if (_on == state)
			return;

		Config.toggleJoystick();
	}
};
