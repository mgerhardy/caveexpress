#pragma once

#include "ui/nodes/UINode.h"

class RateButtonListener: public UINodeListener {
protected:
	const std::string _url;
	IFrontend *_frontend;
	bool _newWindow;
public:
	RateButtonListener (IFrontend *frontend, const std::string& url, bool newWindow = true) :
			_url(url), _frontend(frontend), _newWindow(newWindow)
	{
	}

	void onClick ()
	{
		_frontend->minimize();
		System.openURL(_url, _newWindow);
		Config.getConfigVar("alreadyrated")->setValue("true");
	}
};
