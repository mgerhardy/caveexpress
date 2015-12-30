#include "Darwin.h"
#include "DarwinObjc.h"
#include "common/Application.h"
#include <CoreServices/CoreServices.h>

Darwin::Darwin () :
		Unix()
{
	darwinInit();
}

Darwin::~Darwin ()
{
}

std::string Darwin::getHomeDirectory ()
{
	const std::string& appName = Singleton<Application>::getInstance().getName();
	char* home = darwinGetHomeDirectory(appName);
	if (home == nullptr)
		return "";
	std::string h(home);
	SDL_free(home);
	return h;
}

int Darwin::openURL (const std::string& url, bool) const
{
	CFURLRef cfurl = CFURLCreateWithBytes(nullptr, (const uint8_t *) url.c_str(),
			url.length(), kCFStringEncodingUTF8, nullptr);

	const bool success = LSOpenCFURLRef(cfurl, nullptr) == noErr;
	CFRelease(cfurl);
	return success ? 0 : -1;
}
