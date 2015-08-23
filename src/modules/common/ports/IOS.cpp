#include "IOS.h"
#include "common/Application.h"

IOS::IOS () :
		Unix()
{
}

IOS::~IOS ()
{
}

std::string IOS::getHomeDirectory ()
{
	return "";
}

void IOS::showAds (bool show)
{
	// TODO:
}

int IOS::openURL (const std::string& url, bool) const
{
	const std::string cmd = "open " + url;
	system(cmd.c_str());
	return 0;
}
