#include "IOSObjc.h"
#include <SDL.h>

int iosOpenURL (const std::string& url) {
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
