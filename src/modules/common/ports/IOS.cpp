#include "IOS.h"
#include "Cocoa.h"
#include "common/Application.h"

IOS::IOS () :
		Unix()
{
	nsinit();
}

IOS::~IOS ()
{
}

std::string IOS::getHomeDirectory ()
{
	char* home = nsGetHomeDirectory(Singleton<Application>::getInstance().getName());
	if (home == nullptr)
		return "";
	return home;
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
