#include "IOS.h"
#include "Cocoa.h"
#include "engine/common/Version.h"

IOS::IOS () :
		Unix()
{
	nsinit();
}

IOS::~IOS ()
{
}

void IOS::logError (const std::string& error) const
{
	nslogOutput("ERROR: " + error);
}

void IOS::logOutput (const std::string& string) const
{
	nslogOutput(string);
}

std::string IOS::getHomeDirectory ()
{
	char* home = nsGetHomeDirectory(APPNAME);
	if (home == nullptr)
		return "";
	return home;
}

void IOS::showAds (bool show)
{
	// TODO:
}

int IOS::openURL (const std::string& url) const
{
	const std::string cmd = "open " + url;
	system(cmd.c_str());
    return 0;
}
