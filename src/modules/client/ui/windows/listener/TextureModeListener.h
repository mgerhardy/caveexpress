#pragma once

#include "common/ServiceProvider.h"

class TextureModeListener: public UINodeListener {
private:
	std::string _textureSize;
	IFrontend* _frontend;
	ServiceProvider& _serviceProvider;
public:
	TextureModeListener (const std::string& textureSize, IFrontend* frontend, ServiceProvider& serviceProvider) :
		_textureSize(textureSize), _frontend(frontend), _serviceProvider(serviceProvider)
	{
	}

	void onClick () override
	{
		if (_serviceProvider.getTextureDefinition().getTextureSize() == _textureSize)
			return;
		const TextureDefinition& td = _serviceProvider.getTextureDefinition();
		const int steps = td.getSize();
		UI::get().progressInit(steps, "");
		_serviceProvider.initTextureDefinition(_frontend, _textureSize, &UI::get());
		UI::get().progressDone();
		Config.setTextureSize(_textureSize);
		UI::get().initRestart();
	}
};
