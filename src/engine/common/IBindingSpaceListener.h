#pragma once

class IBindingSpaceListener {
public:
	// called before the BindingSpace is changed - to give a chance to clean up keys
	virtual void resetKeyStates() = 0;

	virtual ~IBindingSpaceListener ()
	{
	}
};
