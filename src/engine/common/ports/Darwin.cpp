#include "Darwin.h"
#include "Cocoa.h"
#include "engine/common/Version.h"

Darwin::Darwin () :
		Unix()
{
	nsinit();
}

Darwin::~Darwin ()
{
}

std::string Darwin::getHomeDirectory ()
{
	char* home = nsGetHomeDirectory(APPNAME);
	if (home == nullptr)
		return "";
	return home;
}

int Darwin::openURL (const std::string& url) const
{
	const std::string cmd = "open \"" + url + "\"";
	return system(cmd.c_str());
}
