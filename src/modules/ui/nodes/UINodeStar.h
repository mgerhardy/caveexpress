#pragma once

#include "ui/nodes/UINode.h"
#include "client/sound/Sound.h"
#include "common/SoundType.h"

class UINodeStar: public UINode {
private:
	int32_t _enableDelay;
	const SoundType& _soundType;
public:
	UINodeStar (IFrontend *frontend, int position, const SoundType& soundType) :
			UINode(frontend), _enableDelay(-1), _soundType(soundType)
	{
		setImage("icon-star-disabled");
		setId("star" + string::toString(position));
	}

	void update (uint32_t deltaTime) override
	{
		UINode::update(deltaTime);
		if (_enableDelay < 0)
			return;

		_enableDelay -= deltaTime;
		if (_enableDelay > 0)
			return;

		setImage("icon-star-enabled");
		if (_soundType.isNone())
			return;
		SoundControl.play(_soundType.getSound());
	}

	void enable (uint32_t delay = 0)
	{
		_enableDelay = delay;
	}

	void disable ()
	{
		_enableDelay = -1;
		setImage("icon-star-disabled");
	}
};
