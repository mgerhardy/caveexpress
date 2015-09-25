#include "IOSObjc.h"
#include <SDL.h>

int iosOpenURL (const std::string& url) const {
#if 0
	@autoreleasepool
	{
		UIApplication *app = [UIApplication sharedApplication];
		NSURL *nsurl = [NSURL URLWithString:@(url.c_str())];

		if ([app canOpenURL:nsurl]) {
			const bool success = [app openURL:nsurl];
			return success ? 0 : -1;
		}
		return -1;
	}
#else
	return -1;
#endif
}

std::string iosGetHomeDirectory() {
#if 0
	@autoreleasepool
	{
		return [NSHomeDirectory() UTF8String];
	}
#else
	return "";
#endif
}
