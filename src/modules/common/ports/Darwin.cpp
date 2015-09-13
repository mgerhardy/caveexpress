#include "Darwin.h"
#include "Cocoa.h"
#include "common/Application.h"
#include <CoreServices/CoreServices.h>

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
