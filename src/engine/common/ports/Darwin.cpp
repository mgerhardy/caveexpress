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

void Darwin::logError (const std::string& error) const
{
	nslogOutput("ERROR: " + error);
}

std::string Darwin::getHomeDirectory ()
{
	char* home = nsGetHomeDirectory(APPNAME);
	if (home == nullptr)
		return "";
	return home;
}

void Darwin::logOutput (const std::string& string) const
{
	nslogOutput(string);
}

int Darwin::openURL (const std::string& url) const
{
	const std::string cmd = "open \"" + url + "\"";
	return system(cmd.c_str());
}
