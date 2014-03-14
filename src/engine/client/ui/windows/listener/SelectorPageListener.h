#pragma once

#include "engine/client/ui/nodes/UINodeSelector.h"

template<class T>
class SelectorPageListener: public UINodeListener {
private:
	UINodeSelector<T> *_mapSelector;
	bool _down;
public:
	SelectorPageListener (UINodeSelector<T> *mapSelector, bool down) :
			_mapSelector(mapSelector), _down(down)
	{
	}

	void onClick () override
	{
		_mapSelector->offset(_down);
	}
};
