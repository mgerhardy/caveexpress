#pragma once

class IMapControl {
public:
	virtual ~IMapControl() {
	}
	virtual bool isPressed () const = 0;

	virtual void hide () = 0;
	virtual void show () = 0;
};
