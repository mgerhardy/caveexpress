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

int Darwin::openURL (const std::string& url, bool) const
{
	CFURLRef cfurl = CFURLCreateWithBytes(nullptr, (const uint8_t *) url.c_str(),
			url.length(), kCFStringEncodingUTF8, nullptr);

	const bool success = LSOpenCFURLRef(cfurl, nullptr) == noErr;
	CFRelease(cfurl);
	return success ? 0 : -1;
}
