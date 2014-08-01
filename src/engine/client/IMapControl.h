#pragma once

class IMapControl {
public:
	virtual ~IMapControl() {
	}
	virtual bool isPressed () const = 0;

	virtual void setVisible (bool visible) = 0;
};
