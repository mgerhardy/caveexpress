#include "Darwin.h"
#include "Cocoa.h"
#include "common/Application.h"

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
	char* home = nsGetHomeDirectory(Singleton<Application>::getInstance().getName().c_str());
	if (home == nullptr)
		return "";
	return home;
}

int Darwin::openURL (const std::string& url, bool) const
{
	const std::string cmd = "open \"" + url + "\"";
	return system(cmd.c_str());
}
