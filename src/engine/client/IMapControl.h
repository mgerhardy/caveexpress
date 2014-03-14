#pragma once

class IMapControl {
public:
	virtual ~IMapControl() {
	}
	virtual bool isPressed () const = 0;
};
